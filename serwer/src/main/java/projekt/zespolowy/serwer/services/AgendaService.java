package projekt.zespolowy.serwer.services;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;
import projekt.zespolowy.serwer.entities.AgendaItemEntity;
import projekt.zespolowy.serwer.entities.UserEntity;
import projekt.zespolowy.serwer.model.AgendaItem;
import projekt.zespolowy.serwer.model.AgendaItemRequest;
import projekt.zespolowy.serwer.model.MessageRequest;
import projekt.zespolowy.serwer.repositories.AgendaItemRepository;
import projekt.zespolowy.serwer.repositories.UserRepository;

import java.util.List;
import java.util.Optional;
import java.util.stream.Collectors;

@Service
public class AgendaService {

    private static final Logger logger = LoggerFactory.getLogger(AgendaService.class);

    private final AgendaItemRepository agendaItemRepository;
    private final UserRepository userRepository;
    private final MessagingService messagingService;

    public AgendaService(AgendaItemRepository agendaItemRepository, 
                         UserRepository userRepository, 
                         MessagingService messagingService) {
        this.agendaItemRepository = agendaItemRepository;
        this.userRepository = userRepository;
        this.messagingService = messagingService;
    }

    public List<AgendaItem> getAllItems() {
        return agendaItemRepository.findAll().stream()
                .map(this::mapToModel)
                .collect(Collectors.toList());
    }

    public Optional<AgendaItem> getItemById(Long id) {
        return agendaItemRepository.findById(id).map(this::mapToModel);
    }

    @Transactional
    public AgendaItem createItem(AgendaItemRequest request) {
        AgendaItemEntity entity = new AgendaItemEntity();
        updateEntityFromRequest(entity, request);
        AgendaItemEntity saved = agendaItemRepository.save(entity);
        return mapToModel(saved);
    }

    @Transactional
    public Optional<AgendaItem> updateItem(Long id, AgendaItemRequest request) {
        return agendaItemRepository.findById(id).map(entity -> {
            boolean criticalChange = isCriticalChange(entity, request);
            updateEntityFromRequest(entity, request);
            AgendaItemEntity saved = agendaItemRepository.save(entity);

            if (criticalChange) {
                notifyAffectedUsers(saved);
            }

            return mapToModel(saved);
        });
    }

    @Transactional
    public void deleteItem(Long id) {
        agendaItemRepository.deleteById(id);
    }

    public List<AgendaItem> getUserSchedule(Long userId) {
        UserEntity user = userRepository.findById(userId)
                .orElseThrow(() -> new RuntimeException("User not found"));
        return user.getScheduledItems().stream()
                .map(this::mapToModel)
                .collect(Collectors.toList());
    }

    @Transactional
    public void addItemToUserSchedule(Long userId, Long itemId) {
        UserEntity user = userRepository.findById(userId)
                .orElseThrow(() -> new RuntimeException("User not found"));
        AgendaItemEntity item = agendaItemRepository.findById(itemId)
                .orElseThrow(() -> new RuntimeException("Agenda item not found"));
        
        user.getScheduledItems().add(item);
        userRepository.save(user);
        
        // Push notification about schedule update
        pushScheduleUpdateNotification(user);
    }

    @Transactional
    public void assignDefaultSchedule(UserEntity user) {
        List<AgendaItemEntity> defaultItems = agendaItemRepository.findAllByIsDefaultTrue();
        if (!defaultItems.isEmpty()) {
            user.getScheduledItems().addAll(defaultItems);
            userRepository.save(user);
            logger.info("Assigned {} default agenda items to user {}", defaultItems.size(), user.getNickname());
        }
    }

    @Transactional
    public void removeItemFromUserSchedule(Long userId, Long itemId) {
        UserEntity user = userRepository.findById(userId)
                .orElseThrow(() -> new RuntimeException("User not found"));
        AgendaItemEntity item = agendaItemRepository.findById(itemId)
                .orElseThrow(() -> new RuntimeException("Agenda item not found"));
        
        user.getScheduledItems().remove(item);
        userRepository.save(user);

        // Push notification about schedule update
        pushScheduleUpdateNotification(user);
    }

    public void pushScheduleUpdateNotification(UserEntity user) {
        if (user.getMacAddress() == null) return;

        MessageRequest pushRequest = new MessageRequest();
        pushRequest.setTargetType(MessageRequest.TargetTypeEnum.MAC);
        pushRequest.setTargetId(user.getMacAddress());
        pushRequest.setCategory(MessageRequest.CategoryEnum.AGENDA_UPDATE);
        pushRequest.setContent("Plan został zaktualizowany");
        messagingService.sendMessage(pushRequest);
    }

    private boolean isCriticalChange(AgendaItemEntity entity, AgendaItemRequest request) {
        if (entity.getLocation() == null || request.getLocation() == null) {
             return entity.getLocation() != request.getLocation();
        }
        return !entity.getLocation().equals(request.getLocation()) ||
               !entity.getStartTime().toString().equals(request.getStartTime()) ||
               !entity.getEndTime().toString().equals(request.getEndTime());
    }

    private void notifyAffectedUsers(AgendaItemEntity item) {
        String notification = String.format("SYNC_AGENDA;%d;%s", item.getId(), item.getLocation());

        MessageRequest messageRequest = new MessageRequest();
        messageRequest.setTargetType(MessageRequest.TargetTypeEnum.BROADCAST);
        messageRequest.setCategory(MessageRequest.CategoryEnum.AGENDA_SYNC);
        messageRequest.setContent(notification);
        
        messagingService.sendMessage(messageRequest);
        logger.info("Broadcasted agenda sync for item {}: {}", item.getId(), notification);
    }

    private AgendaItem mapToModel(AgendaItemEntity entity) {
        AgendaItem model = new AgendaItem();
        model.setId(entity.getId());
        model.setTitle(entity.getTitle());
        model.setDescription(entity.getDescription());
        model.setLocation(entity.getLocation());
        model.setStartTime(entity.getStartTime().toString());
        model.setEndTime(entity.getEndTime().toString());
        model.setTrack(entity.getTrack());
        model.setSpeaker(entity.getSpeaker());
        model.setSpeakerId(entity.getSpeakerId());
        model.setIsDefault(entity.isDefault());
        return model;
    }

    private void updateEntityFromRequest(AgendaItemEntity entity, AgendaItemRequest request) {
        entity.setTitle(request.getTitle());
        entity.setDescription(request.getDescription());
        entity.setLocation(request.getLocation());
        entity.setStartTime(request.getStartTime().toString());
        entity.setEndTime(request.getEndTime().toString());
        entity.setTrack(request.getTrack());
        entity.setSpeaker(request.getSpeaker());
        entity.setSpeakerId(request.getSpeakerId());
        if (request.getIsDefault() != null) {
            entity.setDefault(request.getIsDefault());
        }
    }
}
