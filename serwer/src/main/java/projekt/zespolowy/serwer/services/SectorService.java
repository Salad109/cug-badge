package projekt.zespolowy.serwer.services;

import org.springframework.stereotype.Service;
import projekt.zespolowy.serwer.entities.SectorEntity;
import projekt.zespolowy.serwer.model.Sector;
import projekt.zespolowy.serwer.model.SectorRequest;
import projekt.zespolowy.serwer.repositories.SectorRepository;

import java.util.List;
import java.util.stream.Collectors;

@Service
public class SectorService {

    private final SectorRepository sectorRepository;

    public SectorService(SectorRepository sectorRepository) {
        this.sectorRepository = sectorRepository;
    }

    public List<Sector> getAllSectors() {
        return sectorRepository.findAll().stream()
                .map(this::mapToDto)
                .collect(Collectors.toList());
    }

    public void addSector(SectorRequest sectorRequest) {
        SectorEntity entity = new SectorEntity();
        entity.setName(sectorRequest.getName());
        entity.setGatewayUrl(sectorRequest.getGatewayUrl());
        sectorRepository.save(entity);
    }

    public void deleteSector(Long id) {
        sectorRepository.deleteById(id);
    }

    private Sector mapToDto(SectorEntity entity) {
        Sector dto = new Sector();
        dto.setId(entity.getId());
        dto.setName(entity.getName());
        dto.setGatewayUrl(entity.getGatewayUrl());
        return dto;
    }
}
