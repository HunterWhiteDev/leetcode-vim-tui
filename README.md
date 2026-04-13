# Vim Leetcode TUI

Solve leetcode questions directly in your terminal with vim keybinds!

## Getting Starting

To get started, follow these instructions:

1. Clone the repo and build the project.

```bash
git clone https://github.com/HunterWhiteDev/leetcode-vim-tui.git &&
cd leetcode-vim-tui &&
sudo make
```

2. Create your config file.
   This project relies on using your `LEETCODE_SESSION` and `csrf` token. To set these up create a file at `~/.leetcode/config.conf` with this template:

```
LEETCODE_SESSION=
CSRF_TOKEN=
```

fill in the values from your cookies on the leetcode site.

3. Use the tool.
   From the command line you can use the following commands:

### `leetcode "two sum"`

This will search for all questions with the name "two sum". You can put whatever value you want in quotes.

You will see a screen that looks like this:

![Searvh View](./docs/assets/view1.png)

Use `j` to go up. `k` to go down. `f` selects a problem. When a problem is selected, it will be written to disk at `~/.leetcode/problems/JavaScript/{problemname}.js` (Right now only JS is supported. Multi langage support will be coming soon)

Neovim will then open that file. (Multi editor support will also be coming soon)
Problem explaination will be in a comment block at the top. You will see the explaination and examples before the code block.

### letcode --test {filename}.js

This will test the given file. (default directory is ~/.leetcode/problems/JavaScript/)
This will return print either `success` or `fail` in the terminal.

### letcode --solve {filename}.js

Same as the `--test` command. This will submmit the given file.
This will return print either `success` or `fail` in the terminal.
