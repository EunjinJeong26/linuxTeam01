package com.example.linuxTeam01web.domain.notify;

import com.example.linuxTeam01web.auth.User;
import com.example.linuxTeam01web.auth.UserRepository;
import lombok.RequiredArgsConstructor;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;
import org.springframework.web.server.ResponseStatusException;

@Service
@RequiredArgsConstructor
public class NotifyService {

    private final UserRepository userRepository;

    // PATCH /users/me/notify : 알림 on/off
    @Transactional
    public NotifyResponse updateNotify(Long userId, Boolean enabled) {
        User user = userRepository.findById(userId)
                .orElseThrow(() -> new ResponseStatusException(HttpStatus.NOT_FOUND, "유저를 찾을 수 없습니다."));

        user.setNotify(enabled ? 1 : 0);

        return new NotifyResponse(user.getId(), enabled);
    }

    // GET /users/me/notify : 현재 알림 설정 조회
    public NotifyResponse getNotify(Long userId) {
        User user = userRepository.findById(userId)
                .orElseThrow(() -> new ResponseStatusException(HttpStatus.NOT_FOUND, "유저를 찾을 수 없습니다."));

        return new NotifyResponse(user.getId(), user.getNotify() == 1);
    }
}