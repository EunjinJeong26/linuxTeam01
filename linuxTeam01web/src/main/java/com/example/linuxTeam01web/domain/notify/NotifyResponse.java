package com.example.linuxTeam01web.domain.notify;

import lombok.Getter;

@Getter
public class NotifyResponse {
    private final String username;
    private final Boolean notifyEnabled;

    public NotifyResponse(String username, Boolean notifyEnabled) {
        this.username = username;
        this.notifyEnabled = notifyEnabled;
    }
}