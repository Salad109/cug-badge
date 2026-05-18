package projekt.zespolowy.serwer.controllers;

import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.CrossOrigin;
import org.springframework.web.bind.annotation.RestController;
import projekt.zespolowy.serwer.api.MessagingApi;
import projekt.zespolowy.serwer.model.MessageRequest;
import projekt.zespolowy.serwer.services.MessagingService;

@RestController
public class MessagingController implements MessagingApi {

    private final MessagingService messagingService;

    public MessagingController(MessagingService messagingService) {
        this.messagingService = messagingService;
    }

    @Override
    public ResponseEntity<Void> adminMessagesPost(MessageRequest messageRequest) {
        messagingService.sendMessage(messageRequest);
        return ResponseEntity.accepted().build();
    }
}
