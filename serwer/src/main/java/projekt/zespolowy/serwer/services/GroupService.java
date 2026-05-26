package projekt.zespolowy.serwer.services;

import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;
import projekt.zespolowy.serwer.entities.GroupEntity;
import projekt.zespolowy.serwer.entities.UserEntity;
import projekt.zespolowy.serwer.repositories.GroupRepository;
import projekt.zespolowy.serwer.repositories.UserRepository;

import java.util.List;
import java.util.Optional;

@Service
public class GroupService {

    private final GroupRepository groupRepository;
    private final UserRepository userRepository;

    public GroupService(GroupRepository groupRepository, UserRepository userRepository) {
        this.groupRepository = groupRepository;
        this.userRepository = userRepository;
    }

    public List<GroupEntity> getAllGroups() {
        return groupRepository.findAll();
    }

    public Optional<GroupEntity> getGroupById(Long id) {
        return groupRepository.findById(id);
    }

    @Transactional
    public GroupEntity createGroup(String name, String description) {
        GroupEntity group = new GroupEntity(name, description);
        return groupRepository.save(group);
    }

    @Transactional
    public void deleteGroup(Long id) {
        groupRepository.deleteById(id);
    }

    @Transactional
    public boolean addUserToGroup(Long groupId, Long userId) {
        Optional<GroupEntity> groupOpt = groupRepository.findById(groupId);
        Optional<UserEntity> userOpt = userRepository.findById(userId);

        if (groupOpt.isPresent() && userOpt.isPresent()) {
            GroupEntity group = groupOpt.get();
            UserEntity user = userOpt.get();
            user.getGroups().add(group);
            userRepository.save(user);
            return true;
        }
        return false;
    }

    @Transactional
    public boolean removeUserFromGroup(Long groupId, Long userId) {
        Optional<GroupEntity> groupOpt = groupRepository.findById(groupId);
        Optional<UserEntity> userOpt = userRepository.findById(userId);

        if (groupOpt.isPresent() && userOpt.isPresent()) {
            GroupEntity group = groupOpt.get();
            UserEntity user = userOpt.get();
            user.getGroups().remove(group);
            userRepository.save(user);
            return true;
        }
        return false;
    }
}
