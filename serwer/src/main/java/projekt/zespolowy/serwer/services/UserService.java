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
                .map(entity -> {
                    User dto = new User();
                    dto.setId(entity.getId());
                    dto.setName(entity.getName());
                    dto.setSurname(entity.getSurname());
                    dto.setNickname(entity.getNickname());
                    return dto;
                }).collect(Collectors.toList());
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

    private User mapEntityToDto(UserEntity entity) {
        User dto = new User();
        dto.setId(entity.getId());
        dto.setName(entity.getName());
        dto.setSurname(entity.getSurname());
        dto.setNickname(entity.getNickname());
        dto.seteMail(entity.geteMail());
        dto.setMacAddress(entity.getMacAddress());
        dto.setAccessGroup(entity.getAccessGroup());

        if (entity.getRole() != null) {
            dto.setRole(User.RoleEnum.valueOf(entity.getRole().name()));
        }

        return dto;
    }
}
