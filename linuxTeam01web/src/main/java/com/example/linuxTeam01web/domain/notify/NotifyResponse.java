package com.example.linuxTeam01web.domain.notify;

import lombok.Getter;

@Getter
public class NotifyResponse {
    private final Long userId;
    private final Boolean notifyEnabled;

    public NotifyResponse(Long userId, Boolean notifyEnabled) {
        this.userId = userId;
        this.notifyEnabled = notifyEnabled;
    }
}