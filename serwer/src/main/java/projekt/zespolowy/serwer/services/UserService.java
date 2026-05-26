package projekt.zespolowy.serwer.services;

import java.util.List;
import java.util.Optional;
import java.util.stream.Collectors;

import org.springframework.stereotype.Service;

import projekt.zespolowy.serwer.entities.UserEntity;
import projekt.zespolowy.serwer.model.User;
import projekt.zespolowy.serwer.model.UserRegistrationRequest;
import projekt.zespolowy.serwer.repositories.UserRepository;

@Service
public class UserService {
    private final UserRepository userRepository;

    public UserService(UserRepository userRepository) {
        this.userRepository = userRepository;
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
        newUser.seteMail(user.geteMail());
        newUser.setMacAddress(macAddress);

        // Default values, later can be changed by admin
        newUser.setRole(UserEntity.RoleEnum.USER);
        newUser.setAccessGroup(0);

        userRepository.save(newUser);
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
        userRepository.save(entity);
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

    private void updateEntityFromDto(UserEntity entity, User dto) {
        entity.setName(dto.getName());
        entity.setSurname(dto.getSurname());
        entity.setNickname(dto.getNickname());
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
