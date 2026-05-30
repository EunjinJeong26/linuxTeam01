package com.example.linuxTeam01web.team;

import com.example.linuxTeam01web.auth.User;
import org.springframework.data.jpa.repository.JpaRepository;

import java.util.List;
import java.util.Optional;

public interface TeamMemberRepository extends JpaRepository<TeamMember, Long> {
    boolean existsByUser(User user);

    long countByTeam(Team team);

    // 🌟 새로 추가: 특정 유저가 속한 팀원(TeamMember) 정보 찾기
    Optional<TeamMember> findByUser(User user);

    // 🌟 새로 추가: 특정 팀에 속한 모든 팀원(TeamMember) 목록 찾기
    List<TeamMember> findByTeam(Team team);
}
