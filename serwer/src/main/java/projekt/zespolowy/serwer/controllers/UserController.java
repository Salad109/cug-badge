package projekt.zespolowy.serwer.controllers;

import java.util.List;

import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.RestController;

import projekt.zespolowy.serwer.api.UsersApi;
import projekt.zespolowy.serwer.model.User;
import projekt.zespolowy.serwer.services.UserService;

@RestController
public class UserController implements UsersApi {
    private UserService userService;

    public UserController(UserService userService) {
        this.userService = userService;
    }

    @Override
    public ResponseEntity<List<User>> usersGet() {
        return ResponseEntity.ok(userService.getAll());
    }
    
}
