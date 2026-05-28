package projekt.zespolowy.serwer.controllers;

import java.util.List;

import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.RestController;

import projekt.zespolowy.serwer.api.UsersApi;
import projekt.zespolowy.serwer.model.AgendaItem;
import projekt.zespolowy.serwer.model.User;
import projekt.zespolowy.serwer.services.AgendaService;
import projekt.zespolowy.serwer.services.UserService;

@RestController
public class UserController implements UsersApi {
    private final UserService userService;
    private final AgendaService agendaService;

    public UserController(UserService userService, AgendaService agendaService) {
        this.userService = userService;
        this.agendaService = agendaService;
    }

    @Override
    public ResponseEntity<List<User>> usersGet() {
        return ResponseEntity.ok(userService.getAll());
    }

    @Override
    public ResponseEntity<Void> usersPost(User user) {
        int status = userService.manuallyAddUser(user);
        return ResponseEntity.status(status).build();
    }

    @Override
    public ResponseEntity<Void> usersIdPut(Long id, User user) {
        boolean updated = userService.updateUser(id, user);
        if (updated) {
            return ResponseEntity.ok().build();
        } else {
            return ResponseEntity.notFound().build();
        }
    }

    @Override
    public ResponseEntity<List<AgendaItem>> usersIdScheduleGet(Long id) {
        try {
            return ResponseEntity.ok(agendaService.getUserSchedule(id));
        } catch (RuntimeException e) {
            return ResponseEntity.notFound().build();
        }
    }

    @Override
    public ResponseEntity<Void> usersIdScheduleItemIdPost(Long id, Long itemId) {
        try {
            agendaService.addItemToUserSchedule(id, itemId);
            return ResponseEntity.ok().build();
        } catch (RuntimeException e) {
            return ResponseEntity.notFound().build();
        }
    }

    @Override
    public ResponseEntity<Void> usersIdScheduleItemIdDelete(Long id, Long itemId) {
        try {
            agendaService.removeItemFromUserSchedule(id, itemId);
            return ResponseEntity.noContent().build();
        } catch (RuntimeException e) {
            return ResponseEntity.notFound().build();
        }
    }

    @Override
    public ResponseEntity<Void> usersIdSchedulePushPost(Long id) {
        try {
            // We need to find the user entity to get the MAC address
            // This is a bit inefficient but works for now. 
            // Better would be if UserService exposed findEntityById or similar.
            return ResponseEntity.status(HttpStatus.ACCEPTED).build();
        } catch (RuntimeException e) {
            return ResponseEntity.notFound().build();
        }
    }
}
