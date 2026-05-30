package com.example.linuxTeam01web.domain.log;

import org.springframework.data.jpa.repository.JpaRepository;
import java.util.List;
import java.util.Optional;

public interface LogRepository extends JpaRepository<Log, Long> {

    // devlog show --team : 특정 팀의 특정 날짜 로그 전체 조회
    List<Log> findByTeamIdAndSessionDate(Long teamId, String sessionDate);

    // devlog show --my : 특정 팀의 특정 날짜에서 내 로그만 조회
    List<Log> findByTeamIdAndSessionDateAndUsername(Long teamId, String sessionDate, String username);

    // 중복 체크용 : 같은 슬롯에 이미 로그가 있는지 확인
    Optional<Log> findByUsernameAndTeamIdAndSessionDateAndSessionHour(
        String username, Long teamId, String sessionDate, Integer sessionHour
    );
}