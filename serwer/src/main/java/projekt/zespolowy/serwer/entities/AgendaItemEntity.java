package projekt.zespolowy.serwer.entities;

import jakarta.persistence.*;
import java.time.OffsetDateTime;
import java.util.HashSet;
import java.util.Objects;
import java.util.Set;

import com.fasterxml.jackson.annotation.JsonFormat;

@Entity
@Table(name = "agenda_items")
public class AgendaItemEntity {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    @Column(nullable = false)
    private String title;

    @Column(length = 1000)
    private String description;

    private String location;

    @Column(name = "start_time", nullable = false)
@   JsonFormat(shape = JsonFormat.Shape.STRING, pattern = "yyyy-MM-dd'T'HH:mm")
    private OffsetDateTime startTime;

    @Column(name = "end_time", nullable = false)
    @JsonFormat(shape = JsonFormat.Shape.STRING, pattern = "yyyy-MM-dd'T'HH:mm")
    private OffsetDateTime endTime;

    private String track;

    private String speaker;

    @Column(name = "speaker_id")
    private Long speakerId;

    @Column(name = "is_default", nullable = false)
    private boolean isDefault = false;

    @ManyToMany(mappedBy = "scheduledItems")
    private Set<UserEntity> users = new HashSet<>();

    public AgendaItemEntity() {
    }

    public AgendaItemEntity(String title, OffsetDateTime startTime, OffsetDateTime endTime) {
        this.title = title;
        this.startTime = startTime;
        this.endTime = endTime;
    }

    // Getters and Setters
    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    public String getTitle() {
        return title;
    }

    public void setTitle(String title) {
        this.title = title;
    }

    public String getDescription() {
        return description;
    }

    public void setDescription(String description) {
        this.description = description;
    }

    public String getLocation() {
        return location;
    }

    public void setLocation(String location) {
        this.location = location;
    }

    public OffsetDateTime getStartTime() {
        return startTime;
    }

    public void setStartTime(String startTimeStr) {
        if (startTimeStr != null && !startTimeStr.contains("+") && !startTimeStr.endsWith("Z")) {
            // Doklejamy 'Z' (UTC) lub '+02:00' (Polska latem)
            this.startTime = OffsetDateTime.parse(startTimeStr + ":00+00:00");
        } else {
            this.startTime = OffsetDateTime.parse(startTimeStr);
        }
    }

    public OffsetDateTime getEndTime() {
        return endTime;
    }

    public void setEndTime(String endTimeStr) {
        if (endTimeStr != null && !endTimeStr.contains("+") && !endTimeStr.endsWith("Z")) {
            // Doklejamy 'Z' (UTC) lub '+02:00' (Polska latem)
            this.endTime = OffsetDateTime.parse(endTimeStr + ":00+00:00");
        } else {
            this.endTime = OffsetDateTime.parse(endTimeStr);
        }
    }

    public String getTrack() {
        return track;
    }

    public void setTrack(String track) {
        this.track = track;
    }

    public String getSpeaker() {
        return speaker;
    }

    public void setSpeaker(String speaker) {
        this.speaker = speaker;
    }

    public Long getSpeakerId() {
        return speakerId;
    }

    public void setSpeakerId(Long speakerId) {
        this.speakerId = speakerId;
    }

    public boolean isDefault() {
        return isDefault;
    }

    public void setDefault(boolean aDefault) {
        isDefault = aDefault;
    }

    public Set<UserEntity> getUsers() {
        return users;
    }

    public void setUsers(Set<UserEntity> users) {
        this.users = users;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        AgendaItemEntity that = (AgendaItemEntity) o;
        return Objects.equals(id, that.id);
    }

    @Override
    public int hashCode() {
        return Objects.hash(id);
    }
}
