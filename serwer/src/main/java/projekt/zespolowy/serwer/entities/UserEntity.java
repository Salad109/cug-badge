package projekt.zespolowy.serwer.entities;

import jakarta.persistence.*;
import java.util.Objects;

@Entity
@Table(name = "app_users") 
public class UserEntity {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    @Column(nullable = false)
    private String name;

    @Column(name = "surname")
    private String surname;

    @Column(nullable = false)
    private String nickname;

    @Column(name = "company")
    private String company;

    @Column(name = "position")
    private String position;

    @Column(name = "email")
    private String eMail;

    @Column(name = "mac_address", unique = true)
    private String macAddress;

    public enum RoleEnum {
        USER, VIP, SPEAKER, ADMIN, STAFF
    }

    @Enumerated(EnumType.STRING)
    @Column(nullable = false)
    private RoleEnum role = RoleEnum.USER;

    @Column(name = "access_group")
    private Integer accessGroup;

    @ManyToMany(fetch = FetchType.LAZY)
    @JoinTable(
        name = "user_groups",
        joinColumns = @JoinColumn(name = "user_id"),
        inverseJoinColumns = @JoinColumn(name = "group_id")
    )
    private java.util.Set<GroupEntity> groups = new java.util.HashSet<>();

    @ManyToMany(fetch = FetchType.LAZY)
    @JoinTable(
        name = "user_schedule",
        joinColumns = @JoinColumn(name = "user_id"),
        inverseJoinColumns = @JoinColumn(name = "agenda_item_id")
    )
    private java.util.Set<AgendaItemEntity> scheduledItems = new java.util.HashSet<>();

    // --- Konstruktory ---
    
    public UserEntity() {
        // Pusty konstruktor jest wymagany przez Hibernate/JPA
    }

    public UserEntity(String name, String nickname) {
        this.name = name;
        this.nickname = nickname;
        this.role = RoleEnum.USER; // Domyślna rola przy rejestracji
    }

    // --- Gettery i Settery ---

    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getSurname() {
        return surname;
    }

    public void setSurname(String surname) {
        this.surname = surname;
    }

    public String getNickname() {
        return nickname;
    }

    public void setNickname(String nickname) {
        this.nickname = nickname;
    }

    public String getCompany() {
        return company;
    }

    public void setCompany(String company) {
        this.company = company;
    }

    public String getPosition() {
        return position;
    }

    public void setPosition(String position) {
        this.position = position;
    }

    public String geteMail() {
        return eMail;
    }

    public void seteMail(String eMail) {
        this.eMail = eMail;
    }

    public String getMacAddress() {
        return macAddress;
    }

    public void setMacAddress(String macAddress) {
        this.macAddress = macAddress;
    }

    public RoleEnum getRole() {
        return role;
    }

    public void setRole(RoleEnum role) {
        this.role = role;
    }

    public Integer getAccessGroup() {
        return accessGroup;
    }

    public void setAccessGroup(Integer accessGroup) {
        this.accessGroup = accessGroup;
    }

    public java.util.Set<GroupEntity> getGroups() {
        return groups;
    }

    public void setGroups(java.util.Set<GroupEntity> groups) {
        this.groups = groups;
    }

    public java.util.Set<AgendaItemEntity> getScheduledItems() {
        return scheduledItems;
    }

    public void setScheduledItems(java.util.Set<AgendaItemEntity> scheduledItems) {
        this.scheduledItems = scheduledItems;
    }

    // --- Metody equals i hashCode (Dobre praktyki JPA) ---

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        UserEntity that = (UserEntity) o;
        return Objects.equals(id, that.id) && 
               Objects.equals(macAddress, that.macAddress);
    }

    @Override
    public int hashCode() {
        return Objects.hash(id, macAddress);
    }
}
