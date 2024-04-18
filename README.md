# Table of Contents
1. [Description](https://github.com/emrakyz/sanitize_fs?tab=readme-ov-file#description)
2. [Advantages](https://github.com/emrakyz/sanitize_fs?tab=readme-ov-file#advantages)
3. [Disadvantages](https://github.com/emrakyz/sanitize_fs?tab=readme-ov-file#disadvantages)
4. [Example](https://github.com/emrakyz/sanitize_fs?tab=readme-ov-file#example)
5. [Dependencies](https://github.com/emrakyz/sanitize_fs?tab=readme-ov-file#dependencies)
6. [Installation](https://github.com/emrakyz/sanitize_fs?tab=readme-ov-file#installation)
7. [Usage](https://github.com/emrakyz/sanitize_fs?tab=readme-ov-file#usage)
8. [Why Sanitize Filenames?](https://github.com/emrakyz/sanitize_fs?tab=readme-ov-file#why-sanitize-filenames)
9. [High Level Overview on How It Does Actually Work](https://github.com/emrakyz/sanitize_fs?tab=readme-ov-file#high-level-overview-on-how-it-does-actually-work)
11. [TUI File Manager Integration](https://github.com/emrakyz/sanitize_fs?tab=readme-ov-file#tui-file-manager-integration) 
12. [Video Showcase](https://github.com/emrakyz/sanitize_fs?tab=readme-ov-file#video-showcase)
13. ["It's My Favourite Character. Why Do We Remove It?"](https://github.com/emrakyz/sanitize_fs?tab=readme-ov-file#its-my-favourite-character-why-do-we-remove-it)

# Description
Sanitize file & directory names recursively according to UNIX and URL standards. This means:
- Nothing but lowercased alphanumeric English.
- Spaces, dots and dashes **-->** underscores.
- No consecutive, trailing, leading underscores.
- Keep the extensions intact.
  
The program is extremely fast; mostly instant with normal sized filesystems. If you try unrealistic number of files, directories (1 million+) with unrealistic levels of depth; then it can take about 10 seconds. 

# Advantages
- Utilizes all threads.
- Utilizes a [Thread Lock Mechanism](https://en.wikipedia.org/wiki/Lock_(computer_science)) for synchronization in order to prevent [Race Conditions](https://en.wikipedia.org/wiki/Race_condition) and [Parent-Child](https://en.wikipedia.org/wiki/Directory_(computing)) problems.
- Written in pure [C](https://en.wikipedia.org/wiki/C_(programming_language)#C2Y) with no external dependencies, using [Low Level](https://en.wikipedia.org/wiki/Low-level_programming_language) Linux kernel features.
- Can be statically compiled with agressive [Compiler Optimizations](https://en.wikipedia.org/wiki/Optimizing_compiler) such as `-Ofast` and `LTO` (Link Time Optimizations).
- Skips the files that the current user doesn't own for robustness and speed.
- [Priveledged Root Usage](https://en.wikipedia.org/wiki/Superuser) not allowed for security, safety.
- It skips [Dotfiles](https://en.wikipedia.org/wiki/Hidden_file_and_hidden_directory) (directories and files starting with a dot) encountered at any depth in order to protect configuration files, libraries and hidden files.
- It can never remove files because it uses [renameat2](https://lwn.net/Articles/592952/) function from **The Linux Kernel** with `NO_REPLACE` feature; therefore it is safe.
- It can be directly run on the terminal or it can be invoked by scripts and [TUI File Manager](https://en.wikipedia.org/wiki/Ranger_(file_manager)) keyboard shortcuts. Therefore, it runs fast and is used quickly.
- You can directly rename starting from `/home` with a single command in order to rename simply everything on your system safely (excluding system files, dotfiles, [non-possessed files](https://en.wikipedia.org/wiki/File-system_permissions)), efficiently and in a very fast way.

# Disadvantages
- Obviously won't work on **Windows** and **Mac** but easily portable to **BSD**.
- Doesn't allow custom renaming rules due to safety and consistency. There are better tools that are geared towards custom renaming or simple shell scripts can be written for similar tasks.
- There is no GUI (Graphical User Interface). Though it would be counterproductive to use this program with a GUI.
- Instead of removing all non-English characters; similar characters could be converted to English characters (**é** to **e**) but this is not feasible because:
  
  **a)** There are hundreds of languages and some of the character conversions can be subjective.
  
  **b)** It can be hard to implement because there would be lots of different matching conditions for all of the languages.
  
  **c)** It would make the code significantly bigger and harder to read but I guess it is possible to implement the feature with ANSI lookup tables. 

# Example
`___---...EX😀AM- P😀LE  . DIR😀EC&T😀O^RY..__--/___---...EX😀AMPLE- 😀 . VID😀EO&..__--.mkv`

Becomes:
`example_directory/example_video.mkv`

# Dependencies
Nothing but **The Linux Kernel Version >=3.15** and **Glibc Version >=2.28**. Easily portable to **BSD** and **Musl**.

# Installation
You can directly install a pre-compiled binary from the **Releases** page but the below method is safer and the program will work faster.
1. **Clone the repo:**
   
`git clone "https://github.com/emrakyz/sanitize_fs.git"`

2. **Get into cloned directory:**

`cd "sanitize_fs"`

3. **Statically compile as a single executable binary.**

**With Clang:**
```
clang -static -fno-pic -fno-pie -Ofast -flto=full -pipe -march=native -mtune=native -funroll-loops \
        -Wl,-O3 -Wl,--as-needed -Wl,--gc-sections -Wl,--icf=all \
        "sanitizefs.c" -o "sanitizefs"
```
**With GCC:**
```
gcc -static -fno-pic -fno-pie -Ofast -flto -pipe -march=native -mtune=native -funroll-loops \
        -Wl,-O3 -Wl,--as-needed -Wl,--gc-sections \
        "sanitizefs.c" -o "sanitizefs"
```

4. **Move the file to the desired location (Optional)**

`mv -fv "sanitizefs" "~/.local/bin"` **OR** `sudo mv -fv "sanitizefs" "/usr/bin"`

5. **Remove the cloned unnecessary directory.**

`rm -rfv "../sanitize_fs"`

# Usage
```
USAGE:
        sanitizefs [-dh] [path1] [path2] ...

EXAMPLES:
        sanitizefs EXAMPLE_DIR
        sanitizefs --dry-run "/home/username/EXAMPLE DIR"
        sanitizefs "/home"
        sanitizefs "VIDEO FILE.mkv" "PICTURE.jpg" "DIRECTORY" 

OPTIONS:
        -d, --dry-run        Perform a 'dry run' and exit for testing. Do not rename anything.
        -h, --help           Show this message and exit.
```

# Why Sanitize Filenames
- UNIX-like systems (**Linux** distributions, **BSD** derivatives, **Mac**) heavily favor the described naming conventions. Sanitizing ensures that files and directories interact seamlessly with everything.
- URLs must adhere to specific encoding standards. Sanitizing simplifies their use within web addresses, preventing potential encoding issues and broken links.
- Underscores act as very good word separators, improving file and directory name readability compared to spaces. Spaces used to split command line arguments; they are easier to miss and they require special encoding in URLs and they require escaping on the terminal. On the other hand, dashes are used for CLI programs' flags. Underscores are more unique in this aspect.
- Lowercase standardization eliminates confusion and potential errors when referencing files and directories in scripts or commands.
- It creates all around consistency on your system. You will structure, order, analyze your files in a better way.
- Eliminating special characters prevents issues with file paths in scripts, commands, and programs that might misinterpret them.
- Sanitization helps prevent malicious code injection techniques (like cross-site scripting) that can exploit unexpected characters in URLs.
- Locating, searching, sorting and all other I/O bound operations would be faster and easier. The `find` command, `fzf` command and TUI file managers will definitely be faster with lower latency and you will work with them in an easier, more robust way.
- No matter what website, what operating system or what device you use; these file and directory names will not be problematic and will be easily accessible.
- If you send any files to anyone through network or any other physical method; you will less likely to have problems.

# High Level Overview on How It Does Actually Work
1. Interpret the strings provided as a command line argument and convert them to full paths. 
2. Traverse all directories and files (excluding symbolic links, dotfiles directories and all of their contents and non-possessed files) with a [DFS](https://en.wikipedia.org/wiki/Depth-first_search) (depth-first search) algorithm. DFS method is used to prevent [Race Conditions](https://en.wikipedia.org/wiki/Race_condition) and [Parent-Child](https://en.wikipedia.org/wiki/Directory_(computing)) problems (an upper level directory name being renamed before contents, therefore the contents get lost).
3. Create an [Array Index](https://en.wikipedia.org/wiki/Array_(data_structure)) from the findings by allocating enough memory based on the number of entries that will be renamed. A [Hash Table](https://en.wikipedia.org/wiki/Hash_table) could be used instead of arrays but mainly the data being processed is not that big here. The overhead from the implementation wouldn't outweigh the benefits of the simplicity of arrays.
4. Create threads based on the number of CPUs found on the machine.
5. Split the entries evenly among all of the threads. [Load Balancing](https://en.wikipedia.org/wiki/Load_balancing_(computing)), [Work Stealing](https://en.wikipedia.org/wiki/Work_stealing) or similar algorithms could also be used but since the renaming operation is mostly instant, they wouldn't provide many benefits here.
6. Process entries depth by depth. The program keeps the record of the level of directory depth so it could only process one depth at a time in order to prevent [Race Conditions](https://en.wikipedia.org/wiki/Race_condition), conflictions and [Parent-Child](https://en.wikipedia.org/wiki/Directory_(computing)) problems. A more complex method could be implemented to process even more entries concurrently; but the complexity would probably introduce more problems than benefits.
7. [A Mutex Lock](https://en.wikipedia.org/wiki/Lock_(computer_science)) is used to track all entries. The lock won't be unlocked before every thread finishes processing all of the entries.
8. All of the threads start renaming the entries at the same time by using the character replacement logic (**replace_chars** function).
9. At the end, the lock is unlocked, and the parent directory is closed, and it is renamed individually. The same logic is also used for the indivudual files given to the program as command line arguments. Using threads for them are completely unnecessary since the process would be instant anyways.
10. If the user runs the program with `-d` or `--dry-run` flag, everything would be mostly the same but they will be able to see the before-after results without actually renaming anything.

# TUI File Manager Integration
You can directly use the program inside a directory on TUI File Managers such as [LF](https://github.com/gokcehan/lf), [Ranger](https://github.com/ranger/ranger), [Yazi](https://github.com/sxyazi/yazi) or others. You can add a custom function with a keypress and automatically rename everything inside the current directory.

As an example, for LF file manager, you can create a custom function by assigning a keypress on its configuration file.

The below configuration creates a new command named sanitize and assigns it to "r" keypress: 
```
cmd sanitize ${{
        # Only renames selected files.
	set -f
	sanitizefs ${fx}
}}

map r sanitize
```

You can see the instant renaming operation in the below example video.

# Video Showcase

The first example is unrealistic (a million entries). On an idle machine with 16 threads, the program processes a million entries on 10 level depth, in about 10 seconds.

The second example starting at **00:46**, shows a realistic example using a TUI file manager with an instant operation, with a keypress.

[sanitizefs.webm](https://github.com/emrakyz/sanitize_fs/assets/89175311/c3403780-d28f-4853-8de7-cbd7d5239aad)

# It's My Favourite Character. Why Do We Remove It?

## Why Not Spaces

- They are used as argument separators on shells.
- They require escaping with a backslash.
- They can cause filename splitting.
- They require special encoding on URLs (they appear as "%20").

## Why Not Dashes

- Dashes aren't inherently wrong but they are the second best for the word separation in filenames.
- They are used for command line flags. So, their usage is not unique.
- They are used for subtraction.
- Dashes are used to separate domains, subdomains on URLs.
- They have problematic double-click selections for filenames. For example try to write "word1-word2" on your browser's adress bar. When you double click a word, you can only select either word1 or word2 because they are seen as two arguments and separated by the dash. With underscores you can directly select "word1_word2" with a double click because the underscore doesn't have a special meaning there.
- Dashes are less readable or they can be seen harder, especially with certain fonts.
- `\w` or `\b` characters can't match the full name when dashes used because dashes split `\w` entries. It's same for text editors. When you press `w` on Vim, you jump to the next dash; if you instead use underscores, you jump to the next word because underscores make the name uniformed instead of splitting.

## Why Not Dots
- Dots are used for extension separators, so they have a special meaning.
- They are used to indicate the directory on the shell. Current directory (.), parent directory (..).
- They are used on hidden files, configuration files.
- On URLs, dots have much more important meanings, so it's better to increase uniqueness.
- They are used on IP addresses.
- They are harder to see compared to underscores. The distance is not high enough.

## Why Underscores

- Underscores are common separators in variable and function names in many programming languages.
- They have no special meaning on the shell, they are interpreted as a standard character.
- They clearly separate the words on filenames. The distance is big enough to not miss.
- They require no special encoding on URLs. They are simply seen as "_".
- They can't be mistaken either as a command line flag or a directory location, or a filename extension separator.
- Underscores are considered as word separators by a lot of search engines.
- The filenames won't be split as you experience with dash; you can simply double click to select the complete filename. You can easily use `\w` or `\b` characters with your favourite utilities without worrying about splitting.

## Why Not Special Characters

- Almost all special characters have a specific meaning on the shell. They can show regex, wildcards, variable expansions, command execution and chaining, logical controls and all.
- They also have a specific meaning on URLs.
- They generally require escaping, otherwise they cause problems.
- Especially on URLs, and on file names, this can create a lot of security problems. The specific encoding, and escaping mechanism can be abused.

## Why Not Other Languages

- Most Unix systems, utilize utf-8 English characters by default.
- URLs only support English characters.
- Other languages' specific characters may need extra handling. For example Arabic is reversed or I can't see Japanese characters on my minimal Gentoo setup. They are less universal. Some terminals can't render them properly.
- Search engines or operating systems might not correctly interpret filenames with non-English words, affecting search results and file indexing.
- Characters can have different meanings or appearances based on the language or script, potentially causing confusion.
- Moving files with non-English names between systems with different language support could result in unpredictable outcomes.
- Most things can easily be converted. For example I use C instead of Ç even for Turkish filenames.

## Why Only Lowercased

- It's easier to use lowercased characters on URLs. For example some URLs we use curl with on a TTY are easy to memorize.
- On UNIX and URLs, case sensitivity is important. "File" and "file" can be different. This creates confusion and confliction.
- If you try to use the same files on Windows, you will have problems because Windows can't separate File from file.
- UNIX tools work easier with lowercased characters. It's easier to sort things or it's easier to use tools like grep, sed and all others with lowercased characters. You don't need to think about things you search if they were uppercased or lowercased.
- You don't need to remember the exact capitalization on anything.
