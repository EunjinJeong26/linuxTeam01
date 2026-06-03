package com.example.linuxTeam01web.domain.notify;

import lombok.Getter;
import lombok.NoArgsConstructor;

@Getter
@NoArgsConstructor
public class NotifyRequest {
    private Boolean enabled; // true = on, false = off
}