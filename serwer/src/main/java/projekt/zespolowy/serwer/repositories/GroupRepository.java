package projekt.zespolowy.serwer.repositories;

import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;
import org.springframework.stereotype.Repository;
import projekt.zespolowy.serwer.entities.GroupEntity;

import java.util.List;

@Repository
public interface GroupRepository extends JpaRepository<GroupEntity, Long> {
    
    @Query("SELECT u.macAddress FROM GroupEntity g JOIN g.users u WHERE g.id = :groupId AND u.macAddress IS NOT NULL")
    List<String> findMacAddressesByGroupId(@Param("groupId") Long groupId);
}
