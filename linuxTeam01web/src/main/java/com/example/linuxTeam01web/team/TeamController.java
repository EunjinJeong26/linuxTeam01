package com.example.linuxTeam01web.team;

import org.springframework.http.HttpStatus;
import org.springframework.security.core.annotation.AuthenticationPrincipal;
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
    public TeamDto.CreateResponse createTeam(
            @AuthenticationPrincipal Long userId,
            @RequestBody TeamDto.CreateRequest request) {
        return teamService.createTeam(userId, request);
    }

    // TeamController.java 내부 (기존 클래스 안에 추가)

    @PostMapping("/join")
    @ResponseStatus(HttpStatus.OK) // 명세서 기준 200 OK
    public TeamDto.JoinResponse joinTeam(
            @AuthenticationPrincipal Long userId,
            @RequestBody TeamDto.JoinRequest request) {
        return teamService.joinTeam(userId, request);
    }

    // TeamController.java 내부 (기존 클래스 안에 추가)

    @GetMapping("/me")
    @ResponseStatus(HttpStatus.OK)
    public TeamDto.MyTeamResponse getMyTeam(@AuthenticationPrincipal Long userId) {
        return teamService.getMyTeam(userId);
    }

    // TeamController.java 내부의 기존 메서드들 아래에 추가합니다.

    @DeleteMapping("/me/delete")
    @ResponseStatus(HttpStatus.OK) // 성공 시 200 OK 반환
    public TeamDto.MessageResponse leaveTeam(@AuthenticationPrincipal Long userId) {
        return teamService.leaveTeam(userId);
    }
}
