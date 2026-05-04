package com.example.team01web;

import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping("/test")
public class testController {
    // 1. 기본 GET 요청 테스트
    @GetMapping
    public String test() {
        return "Test OK";
    }

    // 2. PathVariable 테스트
    @GetMapping("/{id}")
    public String getById(@PathVariable int id) {
        return "ID: " + id;
    }

    // 3. Query Parameter 테스트
    @GetMapping("/param")
    public String getParam(@RequestParam String name) {
        return "Hello, " + name;
    }

    // 4. POST 요청 테스트
    @PostMapping
    public String postTest(@RequestBody String body) {
        return "Received: " + body;
    }
}
