package com.example.linuxTeam01web.team;

import com.example.linuxTeam01web.auth.User;
import com.example.linuxTeam01web.auth.UserRepository;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;
import org.springframework.web.server.ResponseStatusException;

import java.util.List;
import java.util.UUID;
import java.util.stream.Collectors;

@Service
public class TeamService {

    private final TeamRepository teamRepository;
    private final TeamMemberRepository teamMemberRepository;
    private final UserRepository userRepository;

    public TeamService(TeamRepository teamRepository, TeamMemberRepository teamMemberRepository, UserRepository userRepository) {
        this.teamRepository = teamRepository;
        this.teamMemberRepository = teamMemberRepository;
        this.userRepository = userRepository;
    }

    @Transactional
    public TeamDto.CreateResponse createTeam(Long userId, TeamDto.CreateRequest request) {
        User user = userRepository.findById(userId)
                .orElseThrow(() -> new ResponseStatusException(HttpStatus.NOT_FOUND, "유저를 찾을 수 없습니다."));

        // 1. 이미 팀에 소속되어 있는지 검증 (409 Conflict)
        if (teamMemberRepository.existsByUser(user)) {
            throw new ResponseStatusException(HttpStatus.CONFLICT, "이미 팀에 소속되어 있습니다.");
        }

        // 2. 8자리 영문+숫자 랜덤 초대 코드 생성 (DECISIONS.md 규칙)
        String inviteCode = UUID.randomUUID().toString().substring(0, 8).toUpperCase();

        // 3. 팀 생성
        Team team = new Team(request.name(), inviteCode);
        Team savedTeam = teamRepository.save(team);

        // 4. 생성자를 리더(leader)로 멤버 테이블에 추가
        TeamMember teamMember = new TeamMember(savedTeam, user, "leader");
        teamMemberRepository.save(teamMember);

        return new TeamDto.CreateResponse(savedTeam.getId(), savedTeam.getName(), savedTeam.getInviteCode());
    }

    // TeamService.java 내부 (기존 클래스 안에 추가)

    @Transactional
    public TeamDto.JoinResponse joinTeam(Long userId, TeamDto.JoinRequest request) {
        User user = userRepository.findById(userId)
                .orElseThrow(() -> new ResponseStatusException(HttpStatus.NOT_FOUND, "유저를 찾을 수 없습니다."));

        // 1. 이미 다른 팀(또는 같은 팀)에 소속되어 있는지 검증 (409)
        if (teamMemberRepository.existsByUser(user)) {
            throw new ResponseStatusException(HttpStatus.CONFLICT, "이미 팀에 소속되어 있습니다.");
        }

        // 2. 초대 코드로 팀 찾기 (404)
        Team team = teamRepository.findByInviteCode(request.invite_code())
                .orElseThrow(() -> new ResponseStatusException(HttpStatus.NOT_FOUND, "존재하지 않는 초대 코드입니다."));

        // 3. 팀 인원이 가득 찼는지 검증 (최대 6명) (409)
        if (teamMemberRepository.countByTeam(team) >= 6) {
            throw new ResponseStatusException(HttpStatus.CONFLICT, "팀 인원이 가득 찼습니다. (최대 6명)");
        }

        // 4. 일반 멤버(member)로 추가
        TeamMember teamMember = new TeamMember(team, user, "member");
        teamMemberRepository.save(teamMember);

        return new TeamDto.JoinResponse(team.getId(), team.getName());
    }

    @Transactional(readOnly = true) // 단순 조회이므로 readOnly = true 설정
    public TeamDto.MyTeamResponse getMyTeam(Long userId) {
        User user = userRepository.findById(userId)
                .orElseThrow(() -> new ResponseStatusException(HttpStatus.NOT_FOUND, "유저를 찾을 수 없습니다."));

        // 1. 유저의 소속 팀 정보 찾기 (없으면 404 에러)
        TeamMember myMembership = teamMemberRepository.findByUser(user)
                .orElseThrow(() -> new ResponseStatusException(HttpStatus.NOT_FOUND, "소속된 팀이 없습니다."));

        Team team = myMembership.getTeam(); // 유저가 속한 진짜 팀 엔티티 추출

        // 2. 해당 팀에 속한 모든 멤버 목록 가져오기
        List<TeamMember> teamMembers = teamMemberRepository.findByTeam(team);

        // 3. 엔티티 목록을 응답용 DTO(MemberInfo) 목록으로 변환
        List<TeamDto.MemberInfo> memberInfos = teamMembers.stream()
                .map(tm -> new TeamDto.MemberInfo(tm.getUser().getId(), tm.getUser().getUsername(), tm.getRole()))
                .collect(Collectors.toList());

        // 4. 최종 DTO 조립 후 반환
        return new TeamDto.MyTeamResponse(
                team.getId(),
                team.getName(),
                team.getInviteCode(),
                memberInfos
        );
    }

    // TeamService.java 내부의 기존 메서드들 아래에 추가합니다.

    @Transactional
    public TeamDto.MessageResponse leaveTeam(Long userId) {
        // 1. 유저 존재 여부 확인
        User user = userRepository.findById(userId)
                .orElseThrow(() -> new ResponseStatusException(HttpStatus.NOT_FOUND, "유저를 찾을 수 없습니다."));

        // 2. 유저의 소속 팀 멤버십 정보 찾기 (없으면 404 에러)
        TeamMember myMembership = teamMemberRepository.findByUser(user)
                .orElseThrow(() -> new ResponseStatusException(HttpStatus.NOT_FOUND, "소속된 팀이 없습니다."));

        Team team = myMembership.getTeam();

        // 3. 역할(role)에 따른 C안 탈퇴 조건 분기
        if ("leader".equals(myMembership.getRole())) {
            // 현재 팀의 전체 인원수 조회
            long memberCount = teamMemberRepository.countByTeam(team);

            if (memberCount > 1) {
                // 3-1. 다른 멤버가 남아있는데 리더가 탈퇴하려는 경우 -> 400 Bad Request로 차단
                throw new ResponseStatusException(HttpStatus.BAD_REQUEST, "다른 멤버가 존재하여 팀 리더는 탈퇴할 수 없습니다. 팀을 해체하려면 다른 멤버들이 먼저 탈퇴해야 합니다.");
            } else {
                // 3-2. 리더 혼자 남은 상태에서 탈퇴하는 경우 -> 멤버 관계 및 팀 엔티티 완전 삭제 (팀 해체)
                teamMemberRepository.delete(myMembership);
                teamRepository.delete(team);
            }
        } else {
            // 4. 일반 멤버(member)인 경우 -> 본인의 팀 소속 행(TeamMember)만 삭제
            teamMemberRepository.delete(myMembership);
        }

        return new TeamDto.MessageResponse("ok");
    }
}
