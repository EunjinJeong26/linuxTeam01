package com.example.linuxTeam01web.domain.log;

import org.springframework.data.jpa.repository.JpaRepository;
import java.util.List;
import java.util.Optional;

public interface LogRepository extends JpaRepository<Log, Long> {

    // devlog show --team
    List<Log> findByTeamIdAndSessionStartingWith(Long teamId, String date);

    // devlog show --my
    List<Log> findByTeamIdAndSessionStartingWithAndUserId(Long teamId, String date, Long userId);

    // 중복 체크
    Optional<Log> findByUserIdAndTeamIdAndSession(Long userId, Long teamId, String session);
}