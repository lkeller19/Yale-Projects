#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum state {READ, POTENTIALCOMMENT, LEADINGTAGSEARCH, NONLEADINGTAGSEARCH, 
            POSSIBLECEND, WRITE, BLOCKIGNORE, POTENTIALBLOCKCOMMENT, SINGLELINEBLOCKCOMMENT, 
            MULTILINEBLOCKCOMMENT, POTENTIALOUTBLOCK, INQUOTE};

int main(int argc, char *argv[])
{
  int in_file = 0;
  int out_file = 0;

  // 0 is default (-l) while 1 is all tags (-a)
  int mode = 0;
  // checks if mode has already been declared. 0 if not.
  int specified = 0;

  for (int i = 0; i < argc; i++)
  {
    // checks if stdin is being reassigned
    if (strcmp(argv[i],"-i") == 0 && in_file != 1)
    {
      freopen(argv[i + 1], "r", stdin);
      in_file = 1;
    }

    // checks if stdout is being reassigned
    if (strcmp(argv[i],"-o") == 0 && out_file != 1)
    {
      freopen(argv[i + 1], "w", stdout);
      out_file = 1;
    }

    // checks if default mode (leading tags only) is being specified
    if (strcmp(argv[i], "-l") == 0 && specified == 0)
    {
      specified = 1;
    }

    // checks if all tags are specified
    if (strcmp(argv[i], "-a") == 0 && specified == 0)
    {
      mode = 1;
      specified = 1;
    }
  }

  // checks to ensure an input file was specified and user input is not enabled
  if (stdin == NULL)
  {
    fprintf(stderr, "Input was not redirected\n");
    return 1;
  }

  enum state curr = READ;
  int c;
  int open_bracket_count;
  // multline comment(/*) is 1, single line (//) is 0
  int comment_type;
  // to keep track of what kind of tag that is currently being looked for (0 for leading, 1 for nonleading)
  int leading_or_nonleading;
  // if writing a tag and there is a forward slash, 0 if not, 1 if so. 
  int line_continue;
  while ((c = getchar()) && c != EOF)
  {
      switch (curr)
      {
          // reads a file when not in a block or comment
          // has three cases of things it might see and then jumps to different states to deal with them
          case READ:
            if (c == '/')
            {
              curr = POTENTIALCOMMENT;
            }
            if (c == '{')
            {
              open_bracket_count = 1;
              curr = BLOCKIGNORE;
            }
            if (c == '"')
            {
              curr = INQUOTE;
            }
            break;

          // checks if first comment indicator is followed by confirmation
          // either confirms the type of comment or jumps back to READ
          case POTENTIALCOMMENT:
            if (c == '*')
            {
              comment_type = 1;
              leading_or_nonleading = 0;
              curr = LEADINGTAGSEARCH;
              break;
            }
            if (c == '/')
            {
              comment_type = 0;
              leading_or_nonleading = 0;
              curr = LEADINGTAGSEARCH;
              break;
            }
            curr = READ;
            break;

          // checks for leading tags
          // also looks out for characters that might indicate somethings else
          case LEADINGTAGSEARCH:
            if (c == '@')
            {
              putchar('@');
              curr = WRITE;
              break;
            }
            if (c == '*' && comment_type == 1)
            {
              curr = POSSIBLECEND;
              break;
            }
            if ((c == '*') || (c == ' '))
            {
              continue;
            }
            if (c == '\n')
            {
              if (comment_type == 0)
              {
                curr = READ;
                break;
              }
              break;
            }
            leading_or_nonleading = 1;
            curr = NONLEADINGTAGSEARCH;
            break;

          // checks for nonleading tags after a leading state has been compromised
          case NONLEADINGTAGSEARCH:
            if (c == '\\')
            {
              line_continue = 1;
              break;
            }
            if (c == '*' && comment_type == 1)
            {
              curr = POSSIBLECEND;
            }
            if (c == '\n')
            {
                if (comment_type == 0)
                {
                  curr = READ;
                  break;
                }
                if (line_continue == 1)
                {
                  line_continue = 0;
                  break;
                }
                leading_or_nonleading = 0;
                curr = LEADINGTAGSEARCH;
                break;
            }
            if (c == '@' && mode == 1)
            {
              putchar('@');
              curr = WRITE;
            }
            break;

          // checks for end of comment
          // uses different variables to keep track of when to do things based on seen input
          case POSSIBLECEND:
            if (c == '*')
            {
              continue;
            }
            if (c == '/')
            {
              curr = READ;
              break;
            }
            if (c == '@')
            {
              if (leading_or_nonleading == 1 || mode == 1)
              {
                putchar('@');
                curr = WRITE;
                break;
              }
              if (leading_or_nonleading == 0)
              {
                putchar('@');
                curr = WRITE;
                break;
              }
              break;
            }
            if (leading_or_nonleading == 0)
            {
              curr = LEADINGTAGSEARCH;
              break;
            }
            curr = NONLEADINGTAGSEARCH;
            break;

          // means that tag has been found and is currently writing a tag
          case WRITE:
            if (c == '\\')
            {
              line_continue = 1;
              break;
            }
            if (c == ' ')
            {
              putchar('\n');
              leading_or_nonleading = 1;
              curr = NONLEADINGTAGSEARCH;
              break;
            }
            if (c == '\n')
            {
              if (line_continue == 1)
              {
                line_continue = 0;
                continue;
              }
              if (comment_type == 0)
              {
                putchar('\n');
                curr = READ;
                break;
              }
              putchar('\n');
              leading_or_nonleading = 0;
              curr = LEADINGTAGSEARCH;
              break;
            }
            if (c == '*' && comment_type == 1)
            {
              putchar('\n');
              curr = POSSIBLECEND;
              break;
            }
            putchar(c);
            break;

          // means code is in a block so can ignore tags
          case BLOCKIGNORE:
            if (c == '{')
            {
              open_bracket_count++;
            }
            if (c == '}')
            {
              open_bracket_count = open_bracket_count - 1;
              if (open_bracket_count == 0)
              {
                
                curr = READ;
                break;
              }
            }
            if (c == '/')
            {
              curr = POTENTIALBLOCKCOMMENT;
              break;
            }
            break;     

          // checking to see if there is a comment inside a block
          case POTENTIALBLOCKCOMMENT:
            if (c == '/')
            {
              curr = SINGLELINEBLOCKCOMMENT;
              break;
            }
            if (c == '*')
            {
              curr = MULTILINEBLOCKCOMMENT;
              break;
            }
            curr = BLOCKIGNORE;
            break;

          // checks for the end of a single line comment in a block
          case SINGLELINEBLOCKCOMMENT:
            if (c == '\n')
            {
              curr = BLOCKIGNORE;
              break;
            }
            break;

          // checks for the potential end of a multiline comment in a block
          case MULTILINEBLOCKCOMMENT:
            if (c == '*')
            {
              curr = POTENTIALOUTBLOCK;
            }
            break;

          // checks for the actual end of a multiline comment in block after potential end is found
          case POTENTIALOUTBLOCK:
            if (c == '*')
            {
              continue;
            }
            if (c == '/')
            {
              curr = BLOCKIGNORE;
              break;
            }
            curr = MULTILINEBLOCKCOMMENT;
            break;

          // means that it is currently reading inside a quote and won't switch back to read till it finds the closing quote
          case INQUOTE:
            if (c == '"')
            {
              curr = READ;
            }
            break;
      }
  }
  fclose(stdin);

  return 0;
}