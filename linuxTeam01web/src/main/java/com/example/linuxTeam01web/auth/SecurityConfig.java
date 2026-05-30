package com.example.linuxTeam01web.auth;

import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.security.config.annotation.web.builders.HttpSecurity;
import org.springframework.security.config.annotation.web.configuration.EnableWebSecurity;
import org.springframework.security.config.annotation.web.configurers.AbstractHttpConfigurer;
import org.springframework.security.crypto.bcrypt.BCryptPasswordEncoder;
import org.springframework.security.crypto.password.PasswordEncoder;
import org.springframework.security.web.SecurityFilterChain;

@Configuration
@EnableWebSecurity
public class SecurityConfig {
    @Bean
    public PasswordEncoder passwordEncoder() {
        return new BCryptPasswordEncoder();
    }

    @Bean
    public SecurityFilterChain filterChain(HttpSecurity http) throws Exception {
        http
                // 1. REST API용 CSRF 비활성화 (이 부분이 적용 안 되면 POST 요청 시 403이 뜹니다)
                .csrf(AbstractHttpConfigurer::disable)

                // 2. 인증 예외 경로 설정
                .authorizeHttpRequests(auth -> auth
                        // 내부 에러 페이지(/error)를 반드시 허용해야 403 대신 정확한 MVC 에러를 확인할 수 있습니다.
                        .requestMatchers("/auth/register", "/auth/login", "/error", "/teams/**").permitAll()
                        .anyRequest().authenticated()
                );

        return http.build();
    }
}