package com.example.linuxTeam01web.auth;

import jakarta.servlet.FilterChain;
import jakarta.servlet.ServletException;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;
import org.springframework.security.authentication.UsernamePasswordAuthenticationToken;
import org.springframework.security.core.context.SecurityContextHolder;
import org.springframework.stereotype.Component;
import org.springframework.web.filter.OncePerRequestFilter;

import java.io.IOException;
import java.util.Collections;

@Component
public class JwtFilter extends OncePerRequestFilter {

    private final JwtUtil jwtUtil;
    private final UserRepository userRepository;

    public JwtFilter(JwtUtil jwtUtil, UserRepository userRepository) {
        this.jwtUtil = jwtUtil;
        this.userRepository = userRepository;
    }

    @Override
    protected void doFilterInternal(HttpServletRequest request, HttpServletResponse response, FilterChain filterChain) throws ServletException, IOException {
        String authorization = request.getHeader("Authorization");

        // 1. 헤더에 Bearer 토큰이 있는지 확인
        if (authorization != null && authorization.startsWith("Bearer ")) {
            String token = authorization.substring(7);

            // 2. 토큰이 유효한지 검증
            if (jwtUtil.validateToken(token)) {
                String username = jwtUtil.getUsername(token);

                // 3. 유저를 찾아 SecurityContext에 저장 (컨트롤러에서 쉽게 쓰도록 userId를 Principal로 저장!)
                userRepository.findByUsername(username).ifPresent(user -> {
                    UsernamePasswordAuthenticationToken authToken =
                            new UsernamePasswordAuthenticationToken(user.getId(), null, Collections.emptyList());
                    SecurityContextHolder.getContext().setAuthentication(authToken);
                });
            }
        }

        // 4. 다음 필터로 넘김
        filterChain.doFilter(request, response);
    }
}
