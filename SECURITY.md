# Security policy

## Supported versions

| Version | Supported |
| ------- | --------- |
| 2.x     | ✅        |
| 1.x     | ❌ (kept for history only) |

## Reporting a vulnerability

Please **do not open a public issue** for a security problem.

Use GitHub's private reporting instead: on the repository page, open the **Security** tab and
choose **Report a vulnerability**. You will get an acknowledgement within a week, and a fix
or an explanation of why the report is not considered a vulnerability.

## Scope

Battleship is an offline, single-player desktop game. It opens no network connection, reads no
file chosen by the user and runs with the user's own privileges. Its inputs are the command
line, the keyboard and the mouse. Realistic issues are therefore memory-safety defects
reachable from those inputs; the test-suite runs under AddressSanitizer and
UndefinedBehaviorSanitizer on every commit to catch them early.
