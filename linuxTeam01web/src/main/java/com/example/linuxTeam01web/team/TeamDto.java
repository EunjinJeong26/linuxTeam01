package com.example.linuxTeam01web.team;
import com.example.linuxTeam01web.auth.User;
import org.springframework.data.jpa.repository.JpaRepository;
import java.util.List;
import java.util.Optional;

public class TeamDto {
    public record CreateRequest(String name) {}
    public record CreateResponse(Long team_id, String name, String invite_code) {}

    public record JoinRequest(String invite_code) {}
    public record JoinResponse(Long team_id, String name) {}

    public record MemberInfo(Long user_id, String username, String role) {}

    public record MyTeamResponse(
            Long team_id,
            String name,
            String invite_code,
            List<MemberInfo> members
    ) {}

    public record MessageResponse(String message) {}
}
