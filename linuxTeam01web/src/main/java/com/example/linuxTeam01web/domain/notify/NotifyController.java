package com.example.linuxTeam01web.domain.notify;

import lombok.RequiredArgsConstructor;
import org.springframework.http.ResponseEntity;
import org.springframework.security.core.annotation.AuthenticationPrincipal;
import org.springframework.security.core.context.SecurityContextHolder;
import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping("/users/me")
@RequiredArgsConstructor
public class NotifyController {

    private final NotifyService notifyService;

    // GET /users/me/notify
    @GetMapping("/notify")
    public ResponseEntity<NotifyResponse> getNotify(@AuthenticationPrincipal Long userId) {
        return ResponseEntity.ok(notifyService.getNotify(userId));
    }

    // PATCH /users/me/notify
    @PatchMapping("/notify")
    public ResponseEntity<NotifyResponse> updateNotify(@AuthenticationPrincipal Long userId, @RequestBody NotifyRequest request) {
        if (request.getEnabled() == null) {
            return ResponseEntity.badRequest().build();
        }

        return ResponseEntity.ok(notifyService.updateNotify(userId, request.getEnabled()));
    }
}