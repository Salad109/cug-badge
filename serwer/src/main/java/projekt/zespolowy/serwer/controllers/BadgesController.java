package projekt.zespolowy.serwer.controllers;

import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.CrossOrigin;
import org.springframework.web.bind.annotation.RestController;

import jakarta.validation.Valid;
import projekt.zespolowy.serwer.api.BadgesApi;
import projekt.zespolowy.serwer.entities.UserEntity;
import projekt.zespolowy.serwer.entities.UserEntity.RoleEnum;
import projekt.zespolowy.serwer.model.User;
import projekt.zespolowy.serwer.model.UserRegistrationRequest;
import projekt.zespolowy.serwer.services.UserService;

@RestController
public class BadgesController implements BadgesApi{
    private final UserService userService;

    public BadgesController (UserService userService) {
        this.userService = userService;
    }

    @Override
    public ResponseEntity<Void> badgesMacAddressRegisterPost(String macAddress, @Valid UserRegistrationRequest userRegistrationRequest) {
        int statusCode = userService.addUser(macAddress, userRegistrationRequest);
        return ResponseEntity.status(statusCode).build();
    }

    @Override
    public ResponseEntity<Void> badgesMacAddressUnlinkPost(String macAddress) {
        boolean isUnlinked = userService.unlinkBadge(macAddress);

        if (isUnlinked) {
            return ResponseEntity.ok().build(); 
        } else {
            return ResponseEntity.notFound().build();
        }
    }

    @Override
    public ResponseEntity<User> badgesMacAddressUserGet(String macAddress) {
        User returnedUser = userService.getUserByMacAddress(macAddress);

        if (returnedUser == null) {
            return ResponseEntity.notFound().build(); 
        }

        return ResponseEntity.ok(returnedUser);
    }

    @Override
    public ResponseEntity<Void> badgesUnlinkAllPost() {
        userService.unlinkAllBadges();
        
        return ResponseEntity.ok().build();
    }
    
}
