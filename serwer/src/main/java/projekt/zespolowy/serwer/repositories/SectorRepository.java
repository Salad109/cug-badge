package projekt.zespolowy.serwer.repositories;

import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;
import projekt.zespolowy.serwer.entities.SectorEntity;

@Repository
public interface SectorRepository extends JpaRepository<SectorEntity, Long> {
}
