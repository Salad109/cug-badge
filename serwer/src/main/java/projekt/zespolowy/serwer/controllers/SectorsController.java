package projekt.zespolowy.serwer.controllers;

import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.CrossOrigin;
import org.springframework.web.bind.annotation.RestController;
import projekt.zespolowy.serwer.api.SectorsApi;
import projekt.zespolowy.serwer.model.Sector;
import projekt.zespolowy.serwer.model.SectorRequest;
import projekt.zespolowy.serwer.services.SectorService;

import java.util.List;

@RestController
public class SectorsController implements SectorsApi {

    private final SectorService sectorService;

    public SectorsController(SectorService sectorService) {
        this.sectorService = sectorService;
    }

    @Override
    public ResponseEntity<List<Sector>> sectorsGet() {
        return ResponseEntity.ok(sectorService.getAllSectors());
    }

    @Override
    public ResponseEntity<Void> sectorsPost(SectorRequest sectorRequest) {
        sectorService.addSector(sectorRequest);
        return ResponseEntity.status(HttpStatus.CREATED).build();
    }

    @Override
    public ResponseEntity<Void> sectorsIdDelete(Long id) {
        sectorService.deleteSector(id);
        return ResponseEntity.noContent().build();
    }
}
