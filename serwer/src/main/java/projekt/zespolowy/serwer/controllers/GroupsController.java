package projekt.zespolowy.serwer.controllers;

import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.RestController;
import projekt.zespolowy.serwer.api.GroupsApi;
import projekt.zespolowy.serwer.entities.GroupEntity;
import projekt.zespolowy.serwer.model.Group;
import projekt.zespolowy.serwer.model.GroupRequest;
import projekt.zespolowy.serwer.services.GroupService;

import jakarta.validation.Valid;
import java.util.List;
import java.util.stream.Collectors;

@RestController
public class GroupsController implements GroupsApi {

    private final GroupService groupService;

    public GroupsController(GroupService groupService) {
        this.groupService = groupService;
    }

    @Override
    public ResponseEntity<List<Group>> groupsGet() {
        List<Group> groups = groupService.getAllGroups().stream()
                .map(this::mapToModel)
                .collect(Collectors.toList());
        return ResponseEntity.ok(groups);
    }

    @Override
    public ResponseEntity<Void> groupsIdDelete(Long id) {
        groupService.deleteGroup(id);
        return ResponseEntity.noContent().build();
    }

    @Override
    public ResponseEntity<Group> groupsIdGet(Long id) {
        return groupService.getGroupById(id)
                .map(this::mapToModel)
                .map(ResponseEntity::ok)
                .orElse(ResponseEntity.notFound().build());
    }

    @Override
    public ResponseEntity<Void> groupsIdMembersUserIdDelete(Long id, Long userId) {
        boolean removed = groupService.removeUserFromGroup(id, userId);
        return removed ? ResponseEntity.noContent().build() : ResponseEntity.notFound().build();
    }

    @Override
    public ResponseEntity<Void> groupsIdMembersUserIdPost(Long id, Long userId) {
        boolean added = groupService.addUserToGroup(id, userId);
        return added ? ResponseEntity.ok().build() : ResponseEntity.notFound().build();
    }

    @Override
    public ResponseEntity<Void> groupsPost(@Valid GroupRequest groupRequest) {
        groupService.createGroup(groupRequest.getName(), groupRequest.getDescription());
        return ResponseEntity.status(HttpStatus.CREATED).build();
    }

    private Group mapToModel(GroupEntity entity) {
        Group model = new Group();
        model.setId(entity.getId());
        model.setName(entity.getName());
        model.setDescription(entity.getDescription());
        if (entity.getUsers() != null) {
            model.setMembers(entity.getUsers().stream()
                    .map(this::mapUserToDtoSimple)
                    .collect(Collectors.toList()));
        }
        return model;
    }

    private projekt.zespolowy.serwer.model.User mapUserToDtoSimple(projekt.zespolowy.serwer.entities.UserEntity entity) {
        projekt.zespolowy.serwer.model.User dto = new projekt.zespolowy.serwer.model.User();
        dto.setId(entity.getId());
        dto.setName(entity.getName());
        dto.setSurname(entity.getSurname());
        dto.setNickname(entity.getNickname());
        dto.seteMail(entity.geteMail());
        dto.setMacAddress(entity.getMacAddress());
        if (entity.getRole() != null) {
            dto.setRole(projekt.zespolowy.serwer.model.User.RoleEnum.valueOf(entity.getRole().name()));
        }
        return dto;
    }
}
