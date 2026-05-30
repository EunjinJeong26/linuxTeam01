package com.example.linuxTeam01web.team;

import org.springframework.http.HttpStatus;
import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping("/teams")
public class TeamController {

    private final TeamService teamService;

    public TeamController(TeamService teamService) {
        this.teamService = teamService;
    }

    @PostMapping
    @ResponseStatus(HttpStatus.CREATED)
    // 🌟 임시 인증 처리: @RequestHeader("X-User-Id")를 통해 어떤 유저가 요청했는지 파악합니다.
    public TeamDto.CreateResponse createTeam(
            @RequestHeader("X-User-Id") Long userId,
            @RequestBody TeamDto.CreateRequest request) {
        return teamService.createTeam(userId, request);
    }

    // TeamController.java 내부 (기존 클래스 안에 추가)

    @PostMapping("/join")
    @ResponseStatus(HttpStatus.OK) // 명세서 기준 200 OK
    public TeamDto.JoinResponse joinTeam(
            @RequestHeader("X-User-Id") Long userId,
            @RequestBody TeamDto.JoinRequest request) {
        return teamService.joinTeam(userId, request);
    }

    // TeamController.java 내부 (기존 클래스 안에 추가)

    @GetMapping("/me")
    @ResponseStatus(HttpStatus.OK)
    public TeamDto.MyTeamResponse getMyTeam(@RequestHeader("X-User-Id") Long userId) {
        return teamService.getMyTeam(userId);
    }

    // TeamController.java 내부의 기존 메서드들 아래에 추가합니다.

    @DeleteMapping("/me/delete")
    @ResponseStatus(HttpStatus.OK) // 성공 시 200 OK 반환
    public TeamDto.MessageResponse leaveTeam(@RequestHeader("X-User-Id") Long userId) {
        return teamService.leaveTeam(userId);
    }
}
