package com.example.linuxTeam01web.websocket;

import com.example.linuxTeam01web.domain.log.LogResponse;
import lombok.RequiredArgsConstructor;
import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.stereotype.Component;

@Component
@RequiredArgsConstructor
public class LogWebSocketHandler {

    private final SimpMessagingTemplate messagingTemplate;

    // 로그 저장 후 같은 팀원에게 실시간 푸시
    // LogService에서 로그 저장 완료 후 이 메서드 호출
    public void broadcastLog(Long teamId, LogResponse logResponse) {
        // /topic/team/{teamId} 구독 중인 클라이언트에게 전송
        messagingTemplate.convertAndSend("/topic/team/" + teamId, logResponse);
    }
}