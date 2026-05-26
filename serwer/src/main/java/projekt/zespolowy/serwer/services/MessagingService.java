package projekt.zespolowy.serwer.services;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;
import org.springframework.web.client.RestTemplate;
import projekt.zespolowy.serwer.entities.SectorEntity;
import projekt.zespolowy.serwer.model.MessageRequest;
import projekt.zespolowy.serwer.repositories.GroupRepository;
import projekt.zespolowy.serwer.repositories.SectorRepository;

import java.util.List;
import java.util.Optional;

@Service
public class MessagingService {

    private static final Logger logger = LoggerFactory.getLogger(MessagingService.class);
    private final SectorRepository sectorRepository;
    private final GroupRepository groupRepository;
    private final RestTemplate restTemplate;

    public MessagingService(SectorRepository sectorRepository, GroupRepository groupRepository) {
        this.sectorRepository = sectorRepository;
        this.groupRepository = groupRepository;
        this.restTemplate = new RestTemplate();
    }

    public void sendMessage(MessageRequest messageRequest) {
        switch (messageRequest.getTargetType()) {
            case BROADCAST:
                broadcastMessage(messageRequest);
                break;
            case SECTOR:
                sendToSector(messageRequest);
                break;
            case MAC:
                sendToMac(messageRequest);
                break;
            case GROUP:
                sendToGroup(messageRequest);
                break;
        }
    }

    private void broadcastMessage(MessageRequest request) {
        List<SectorEntity> sectors = sectorRepository.findAll();
        for (SectorEntity sector : sectors) {
            forwardToGateway(sector.getGatewayUrl(), request);
        }
    }

    private void sendToSector(MessageRequest request) {
        try {
            Long sectorId = Long.parseLong(request.getTargetId());
            Optional<SectorEntity> sector = sectorRepository.findById(sectorId);
            sector.ifPresent(entity -> forwardToGateway(entity.getGatewayUrl(), request));
        } catch (NumberFormatException e) {
            logger.error("Invalid sector ID: {}", request.getTargetId());
        }
    }

    private void sendToGroup(MessageRequest request) {
        try {
            Long groupId = Long.parseLong(request.getTargetId());
            List<String> macAddresses = groupRepository.findMacAddressesByGroupId(groupId);
            
            for (String mac : macAddresses) {
                MessageRequest macRequest = new MessageRequest();
                macRequest.setTargetType(MessageRequest.TargetTypeEnum.MAC);
                macRequest.setTargetId(mac);
                macRequest.setContent(request.getContent());
                macRequest.setCategory(request.getCategory());
                sendToMac(macRequest);
            }
        } catch (NumberFormatException e) {
            logger.error("Invalid group ID: {}", request.getTargetId());
        }
    }

    private void sendToMac(MessageRequest request) {
        // For now, if targeting a MAC, we broadcast to all gateways
        // because we don't track which MAC is in which sector.
        // The gateway or the badge itself will filter based on MAC.
        broadcastMessage(request);
    }

    private void forwardToGateway(String url, MessageRequest request) {
        try {
            logger.info("Forwarding message to gateway {}: content='{}'", url, request.getContent());
            // In a real scenario, you'd send a specific DTO that the gateway expects
            restTemplate.postForEntity(url, request, Void.class);
        } catch (Exception e) {
            logger.error("Failed to send message to gateway at {}: {}", url, e.getMessage());
        }
    }
}
