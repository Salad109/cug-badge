package projekt.zespolowy.serwer.services;

import java.util.List;
import java.util.Optional;
import java.util.stream.Collectors;

import org.springframework.stereotype.Service;
import com.fasterxml.jackson.databind.ObjectMapper;

import projekt.zespolowy.serwer.entities.UserEntity;
import projekt.zespolowy.serwer.model.User;
import projekt.zespolowy.serwer.model.UserRegistrationRequest;
import projekt.zespolowy.serwer.repositories.UserRepository;

@Service
public class UserService {
    private final UserRepository userRepository;
    private final AgendaService agendaService;
    private final MessagingService messagingService;
    private final ObjectMapper objectMapper;

    public UserService(UserRepository userRepository, AgendaService agendaService, MessagingService messagingService, ObjectMapper objectMapper) {
        this.userRepository = userRepository;
        this.agendaService = agendaService;
        this.messagingService = messagingService;
        this.objectMapper = objectMapper;
    }

    public List<User> getAll() {
        return userRepository.findAll().stream()
                .map(this::mapEntityToDto)
                .collect(Collectors.toList());
    }

    public int addUser(String macAddress, UserRegistrationRequest user) {
        if (userRepository.existsByMacAddress(macAddress)) {
            return 409;
        }

        UserEntity newUser = new UserEntity();

        // User information
        newUser.setName(user.getName());
        newUser.setSurname(user.getSurname());
        newUser.setNickname(user.getNickname());
        newUser.setCompany(user.getCompany());
        newUser.setPosition(user.getPosition());
        newUser.seteMail(user.geteMail());
        newUser.setMacAddress(macAddress);

        // Default values, later can be changed by admin
        newUser.setRole(UserEntity.RoleEnum.USER);
        newUser.setAccessGroup(0);

        UserEntity savedUser = userRepository.save(newUser);
        agendaService.assignDefaultSchedule(savedUser);
        
        sendWelcomeMessage(savedUser);
        
        return 201;
    }

    public User getUserByMacAddress(String macAddress) {
        Optional<UserEntity> userOptional = userRepository.findByMacAddress(macAddress);
        if (userOptional.isPresent()) {
            return mapEntityToDto(userOptional.get());
        } else {
            return null; 
        }
    }

    public boolean unlinkBadge(String macAddress) {
        Optional<UserEntity> userOptional = userRepository.findByMacAddress(macAddress);
        
        if (userOptional.isPresent()) {
            userRepository.delete(userOptional.get());
            return true;
        }
        
        return false;
    }

    public void unlinkAllBadges() {
        userRepository.deleteAll();
    }

    public int manuallyAddUser(User user) {
        if (user.getMacAddress() != null && userRepository.existsByMacAddress(user.getMacAddress())) {
            return 409;
        }
        UserEntity entity = new UserEntity();
        updateEntityFromDto(entity, user);
        UserEntity savedUser = userRepository.save(entity);
        agendaService.assignDefaultSchedule(savedUser);
        return 201;
    }

    public boolean updateUser(Long id, User user) {
        Optional<UserEntity> optional = userRepository.findById(id);
        if (optional.isPresent()) {
            UserEntity entity = optional.get();
            updateEntityFromDto(entity, user);
            userRepository.save(entity);
            return true;
        }
        return false;
    }

    private void sendWelcomeMessage(UserEntity user) {
        projekt.zespolowy.serwer.model.MessageRequest message = new projekt.zespolowy.serwer.model.MessageRequest();
        message.setTargetType(projekt.zespolowy.serwer.model.MessageRequest.TargetTypeEnum.MAC);
        message.setTargetId(user.getMacAddress());
        message.setCategory(projekt.zespolowy.serwer.model.MessageRequest.CategoryEnum.INFO);

        try {
            UserRegistrationRequest registrationData = new UserRegistrationRequest();
            registrationData.setName(user.getName());
            registrationData.setSurname(user.getSurname());
            registrationData.setNickname(user.getNickname());
            registrationData.setCompany(user.getCompany());
            registrationData.setPosition(user.getPosition());
            registrationData.seteMail(user.geteMail());

            String jsonContent = objectMapper.writeValueAsString(registrationData);
            message.setContent(jsonContent);
            messagingService.sendMessage(message);
        } catch (Exception e) {
            System.err.println("Error serializing registration message: " + e.getMessage());
        }
    }

    private String valOrEmpty(String val) {
        return (val != null) ? val : "";
    }

    private void updateEntityFromDto(UserEntity entity, User dto) {
        entity.setName(dto.getName());
        entity.setSurname(dto.getSurname());
        entity.setNickname(dto.getNickname());
        entity.setCompany(dto.getCompany());
        entity.setPosition(dto.getPosition());
        entity.seteMail(dto.geteMail());
        entity.setMacAddress(dto.getMacAddress());
        if (dto.getRole() != null) {
            entity.setRole(UserEntity.RoleEnum.valueOf(dto.getRole().name()));
        }
    }

    private User mapEntityToDto(UserEntity entity) {
        User dto = new User();
        dto.setId(entity.getId());
        dto.setName(entity.getName());
        dto.setSurname(entity.getSurname());
        dto.setNickname(entity.getNickname());
        dto.setCompany(entity.getCompany());
        dto.setPosition(entity.getPosition());
        dto.seteMail(entity.geteMail());
        dto.setMacAddress(entity.getMacAddress());

        if (entity.getRole() != null) {
            dto.setRole(User.RoleEnum.valueOf(entity.getRole().name()));
        }

        if (entity.getGroups() != null) {
            dto.setGroups(entity.getGroups().stream()
                    .map(this::mapGroupToDtoSimple)
                    .collect(Collectors.toList()));
        }

        return dto;
    }

    private projekt.zespolowy.serwer.model.Group mapGroupToDtoSimple(projekt.zespolowy.serwer.entities.GroupEntity entity) {
        projekt.zespolowy.serwer.model.Group dto = new projekt.zespolowy.serwer.model.Group();
        dto.setId(entity.getId());
        dto.setName(entity.getName());
        dto.setDescription(entity.getDescription());
        return dto;
    }
}
