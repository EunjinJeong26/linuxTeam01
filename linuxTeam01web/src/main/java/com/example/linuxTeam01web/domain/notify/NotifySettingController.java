package com.example.linuxTeam01web.domain.notify;

import lombok.RequiredArgsConstructor;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping("/users/me")
@RequiredArgsConstructor
public class NotifySettingController {

    private final NotifySettingService notifySettingService;

    // GET /users/me/notify : 현재 알림 설정 조회
    @GetMapping("/notify")
    public ResponseEntity<NotifyResponse> getNotify(
            @RequestHeader("X-Username") String username) {

        Boolean status = notifySettingService.getNotifyStatus(username);
        return ResponseEntity.ok(new NotifyResponse(username, status));
    }

    // PATCH /users/me/notify : 알림 on/off
    @PatchMapping("/notify")
    public ResponseEntity<NotifyResponse> updateNotify(
            @RequestHeader("X-Username") String username,
            @RequestBody NotifyRequest request) {

        // null 체크
        if (request.getEnabled() == null) {
            return ResponseEntity.badRequest().build();
        }

        NotifySetting setting = notifySettingService.updateNotify(username, request.getEnabled());
        return ResponseEntity.ok(new NotifyResponse(setting.getUsername(), setting.getNotifyEnabled()));
    }
}