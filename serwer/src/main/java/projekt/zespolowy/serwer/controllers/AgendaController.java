package projekt.zespolowy.serwer.controllers;

import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.RestController;
import projekt.zespolowy.serwer.api.AgendaApi;
import projekt.zespolowy.serwer.model.AgendaItem;
import projekt.zespolowy.serwer.model.AgendaItemRequest;
import projekt.zespolowy.serwer.services.AgendaService;

import java.util.List;

@RestController
public class AgendaController implements AgendaApi {

    private final AgendaService agendaService;

    public AgendaController(AgendaService agendaService) {
        this.agendaService = agendaService;
    }

    @Override
    public ResponseEntity<List<AgendaItem>> agendaItemsGet() {
        return ResponseEntity.ok(agendaService.getAllItems());
    }

    @Override
    public ResponseEntity<AgendaItem> agendaItemsPost(AgendaItemRequest agendaItemRequest) {
        return ResponseEntity.status(HttpStatus.CREATED).body(agendaService.createItem(agendaItemRequest));
    }

    @Override
    public ResponseEntity<Void> agendaItemsIdDelete(Long id) {
        agendaService.deleteItem(id);
        return ResponseEntity.noContent().build();
    }

    @Override
    public ResponseEntity<AgendaItem> agendaItemsIdGet(Long id) {
        return agendaService.getItemById(id)
                .map(ResponseEntity::ok)
                .orElse(ResponseEntity.notFound().build());
    }

    @Override
    public ResponseEntity<AgendaItem> agendaItemsIdPut(Long id, AgendaItemRequest agendaItemRequest) {
        return agendaService.updateItem(id, agendaItemRequest)
                .map(ResponseEntity::ok)
                .orElse(ResponseEntity.notFound().build());
    }
}
