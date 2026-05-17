package projekt.zespolowy.serwer.repositories;

import java.util.Optional;

import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;
import projekt.zespolowy.serwer.entities.UserEntity;

@Repository
public interface UserRepository extends JpaRepository<UserEntity, Long> {
    boolean existsByMacAddress(String macAddress);
    Optional<UserEntity> findByMacAddress(String macAddress);
}
