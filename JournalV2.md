# Journal Volume 2, September 10, 2025

Feb 24, 2022 at 3:54 PM was when I made the first commit (this date is very ominous). The actual name OS/90 came at least weeks before that. I had my first and oldest laptop back then and I haven't checked when the files were created yet.

Over 3.5 years of development. At this rate though, I think I have a good strategy and know much more than I did before. I hope I can release a finished product of some sort before 10 years or something.

Not that it really matters much. OS/90 in its current planned form is really nothing close to how I could have imagined it during junior year of high school.

I was actually 16 when I started this, and did OS dev and assembly stuff when I was 15.

## Continued

> I respect TempleOS, but OS/90 is the DREAM, remember? I need 256 colors for that, right?
> I opened a very old 90's thinkpad and the operating system on it looked kind of like minecraft. The cursor was similar. The windows and everything had this textured dirt and stone color.

### Text mode cursor?

I am not sure where this is actually stored. In graphical modes it does not exist, but still needs to go somewhere.

The CON device may use some kind of information field for this.

No, actually, the CON device really just calls INT 10H or it is not even the target of the operation due to redirection.

The BIOS stores the cursor location (where the next char is printed). It can be moved too.

INT 10H and other things will be captured so I don't think this is a problem.

Windows handles CON potentially returning two bytes when ^P is pressed, which has a race condition problem. I would prefer to not use real mode code at all for CON if possible.

I could allow drivers to be controlled by protected mode using a callback for the strategy/interrupt points.

## CON device in detail

DOS is allowed to call CON whenever it wants to. Same logic as with other things. DOS does not need to use its own public interface.

Consider something like the infamous critical error message. DOS is allowed to write to the CON device by just sending a data packet to it.

I think it was mainly a problem for Win386 where it had to manually enter a critical section on behalf of the program and disable IO emulation so that it could execute an INT call. CON did not do this, so there was some internal interface and a VxD that handled all this.

Handling an INT while preemption is disabled is actually possible and necessary. I have debated how V86 will work, but it is necessary to hook and capture INT calls even if preemption is disabled and SV86 is running. As long as non-reentrant MT-unsafe RM code isn't running, there is no real problem.

The only problem with CON is that ^P sends two bytes, the first being null. On windows this created an atomicity problem that had to be fixed manually using some hacks in the driver itself.

FreeDOS CON does not appear to use any such interface.

Not really a big deal though. I can fix any issues later since it is a minor problem.

## Testing strategy


# September 14
