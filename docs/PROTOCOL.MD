# OS/90 Development Protocol

The rules are:
- Never write a single line of code that cannot be tested.

- The code must compile and run at all times. Never add a single line that does not compile and run.

- Write as much code as possible in userspace and integrate into OS/90 later

- Code that may be useful later should be saved in a reserve folder for later use.

- Any components added to OS/90 after unit testing must be certified 100% bug free.

- Features are cringe. They exponentially increase development time and should be added sparingly with a very good reason to do so.

- Use C until there is clear evidence that assembly is better.
