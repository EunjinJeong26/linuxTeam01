package com.example.linuxTeam01web.domain.notify;

import lombok.RequiredArgsConstructor;
import org.springframework.http.ResponseEntity;
import org.springframework.security.core.context.SecurityContextHolder;
import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping("/users/me")
@RequiredArgsConstructor
public class NotifyController {

    private final NotifyService notifyService;

    // GET /users/me/notify
    @GetMapping("/notify")
    public ResponseEntity<NotifyResponse> getNotify() {
        Long userId = (Long) SecurityContextHolder.getContext()
                .getAuthentication().getPrincipal();
        return ResponseEntity.ok(notifyService.getNotify(userId));
    }

    // PATCH /users/me/notify
    @PatchMapping("/notify")
    public ResponseEntity<NotifyResponse> updateNotify(@RequestBody NotifyRequest request) {
        Long userId = (Long) SecurityContextHolder.getContext()
                .getAuthentication().getPrincipal();

        if (request.getEnabled() == null) {
            return ResponseEntity.badRequest().build();
        }

        return ResponseEntity.ok(notifyService.updateNotify(userId, request.getEnabled()));
    }
}