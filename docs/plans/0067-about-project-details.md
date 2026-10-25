# Feature: current About information and public contact

Status: acceptance defined; native validation deferred
Target: v2.1.0

Show the build's actual version, Linux/macOS description, approved public credit
“Afshin Kashani, PhD, P.Eng”, GPL license and independent-project disclosure.
Provide explicit Website and Contact / report an issue buttons using the public
project site and GitHub Issues. No email address is inferred or published.

Acceptance: opening About sends no request; only deliberate button activation
opens a fixed HTTPS destination. Browser launch failure leaves a readable URL.
F1, keyboard focus, Escape/OK, light/dark and minimum-size layout remain usable.
Document pixels/history are unaffected. Extend the URL-handler test without
making real browser/network requests; defer native execution/build to final pass.
