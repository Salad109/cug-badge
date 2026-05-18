package projekt.zespolowy.serwer.controllers;

import java.util.List;

import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.RestController;

import projekt.zespolowy.serwer.api.UsersApi;
import projekt.zespolowy.serwer.model.User;
import projekt.zespolowy.serwer.services.UserService;

@RestController
public class UserController implements UsersApi {
    private final UserService userService;

    public UserController(UserService userService) {
        this.userService = userService;
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
}
