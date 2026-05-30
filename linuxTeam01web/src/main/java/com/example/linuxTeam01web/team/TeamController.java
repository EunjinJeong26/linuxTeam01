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

    @PostMapping("/join")
    @ResponseStatus(HttpStatus.OK) // 명세서 기준 200 OK
    public TeamDto.JoinResponse joinTeam(
            @AuthenticationPrincipal Long userId,
            @RequestBody TeamDto.JoinRequest request) {
        return teamService.joinTeam(userId, request);
    }

    @GetMapping("/me")
    @ResponseStatus(HttpStatus.OK)
    public TeamDto.MyTeamResponse getMyTeam(@AuthenticationPrincipal Long userId) {
        return teamService.getMyTeam(userId);
    }

    @DeleteMapping("/me/delete")
    @ResponseStatus(HttpStatus.OK) // 성공 시 200 OK 반환
    public TeamDto.MessageResponse leaveTeam(@AuthenticationPrincipal Long userId) {
        return teamService.leaveTeam(userId);
    }
}
