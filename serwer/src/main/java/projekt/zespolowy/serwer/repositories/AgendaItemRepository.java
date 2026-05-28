package projekt.zespolowy.serwer.repositories;

import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;
import org.springframework.stereotype.Repository;
import projekt.zespolowy.serwer.entities.AgendaItemEntity;

import java.util.List;

@Repository
public interface AgendaItemRepository extends JpaRepository<AgendaItemEntity, Long> {

    @Query("SELECT u.macAddress FROM AgendaItemEntity a JOIN a.users u WHERE a.id = :agendaItemId AND u.macAddress IS NOT NULL")
    List<String> findUserMacsByAgendaItemId(@Param("agendaItemId") Long agendaItemId);

    List<AgendaItemEntity> findAllByIsDefaultTrue();
}
