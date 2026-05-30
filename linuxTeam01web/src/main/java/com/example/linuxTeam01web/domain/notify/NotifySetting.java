package com.example.linuxTeam01web.domain.notify;

import jakarta.persistence.*;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

@Entity
@Table(name = "notify_settings")
@Getter
@Setter
@NoArgsConstructor
public class NotifySetting {

    @Id
    private String username;  // 유저당 1개라서 username이 PK

    private Boolean notifyEnabled;  // true = 알림 on, false = 알림 off
}