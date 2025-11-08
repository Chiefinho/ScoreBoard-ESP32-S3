## Current Status

- Added a toggleable scrolling text demo controlled over serial (`s` to start, `0` to stop).
- Scrolling routine runs inside `loop()` when active and reuses the existing matrix instance.
- Startup banner aligned to top-left to avoid overlapping with scroll region.

## Next Ideas

- Parameterize scroll text and speed via serial commands.
- Persist scroll state across resets if needed.
- Capture a short video/GIF of the scroll test for documentation.

