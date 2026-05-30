package com.example.linuxTeam01web.domain.notify;

import org.springframework.data.jpa.repository.JpaRepository;
import java.util.Optional;

public interface NotifySettingRepository extends JpaRepository<NotifySetting, String> {
    Optional<NotifySetting> findByUsername(String username);
}