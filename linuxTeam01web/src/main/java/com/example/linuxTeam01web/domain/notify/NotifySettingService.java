package com.example.linuxTeam01web.domain.notify;

import lombok.RequiredArgsConstructor;
import org.springframework.stereotype.Service;

@Service
@RequiredArgsConstructor
public class NotifySettingService {

    private final NotifySettingRepository notifySettingRepository;

    // PATCH /users/me/notify : 알림 on/off 토글
    public NotifySetting updateNotify(String username, Boolean enabled) {

        // 이미 설정이 있으면 업데이트, 없으면 새로 생성
        NotifySetting setting = notifySettingRepository.findByUsername(username)
                .orElse(new NotifySetting());

        setting.setUsername(username);
        setting.setNotifyEnabled(enabled);

        return notifySettingRepository.save(setting);
    }

    // 현재 알림 설정 조회
    public Boolean getNotifyStatus(String username) {
        return notifySettingRepository.findByUsername(username)
                .map(NotifySetting::getNotifyEnabled)
                .orElse(false); // 설정 없으면 기본값 off
    }
}