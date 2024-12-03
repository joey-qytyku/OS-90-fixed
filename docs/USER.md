# Userspace API Ideas

## Components

- Dynamic linking
- File IO and paths
-


## Paths

Paths work like they do in Java NIO. Internally, paths are just linked lists combined with pascal strings in a way that optimally conveys the path and all of its elements.

Path objects can be joined together to form other paths. They also have a copy-on-write feature that automatically duplicates a path object if changed for one superpath.

```
int main()
{
	hPATH p_subdirs[2] = { HPATH_INVALID, HPATH_INVALID };
	hPATH p_home;
	hPATH p_final;
	hFILE output;
	int choice;

	p_home = PathCreate("$HOME");
	PathTravelWildcard(p_subdirs, "/{DOCS, DESKTOP}");

	std::cout << "Save the file to DOCS or DESKTOP? [0/1]";
	i << std::cin;

	p_final = PathAssemble("p/p/s", p_home, p_subdirs[choice], "FILE.TXT");

	output = FileOpen(p_final, FILE_RDWR | FILE_EXCL | FILE_CREATE);

	FileWrite(file, "Hello, world!");

	PathDeleteTree
}
```

This is a suboptimal example to show what it can do.

The features:
- Assembling paths with format string, dyadic concatentation, array concatenation.
- Using environment variables in the strings
- Deleting paths that are not being used by any other ones
- Paths are automatically validated to prevent circles.

Also I can add the option to use a format string to extract parts of the path or change them.

```
PathExtractFmt(const char *fmt, ...);
PathReplaceFmt(const char *fmt, ...);
```
