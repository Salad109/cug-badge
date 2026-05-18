package projekt.zespolowy.serwer.entities;

import jakarta.persistence.*;
import java.util.Objects;

@Entity
@Table(name = "sectors")
public class SectorEntity {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    @Column(nullable = false)
    private String name;

    @Column(name = "gateway_url", nullable = false)
    private String gatewayUrl;

    public SectorEntity() {
    }

    public SectorEntity(String name, String gatewayUrl) {
        this.name = name;
        this.gatewayUrl = gatewayUrl;
    }

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

    public String getGatewayUrl() {
        return gatewayUrl;
    }

    public void setGatewayUrl(String gatewayUrl) {
        this.gatewayUrl = gatewayUrl;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        SectorEntity that = (SectorEntity) o;
        return Objects.equals(id, that.id);
    }

    @Override
    public int hashCode() {
        return Objects.hash(id);
    }
}
