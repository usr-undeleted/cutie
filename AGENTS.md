# cutie project AI context

## Overview
Cutie (Core UTIlities Environment) is a collection of CLI commands meant to be simple and compact following the KISS principle.  
README.md contains short plans and more explanation on the project.  

## Concepts
- Very similar features, with almost exact purposes, should be grouped together neatly.
- Comments in code are short, and omly used to explain large functions and/or confusing pieces of code.
- Focus on efficiency and speed when writing code. If an alternative is even slightly faster, use it, as long as it integrates nicely with existant code.
- Consistent naming for varibales (myVar, myFunction()).
- Consistent formatting following existent code.
- Crucial features will always prioritize non-necessary additions.
- If a shell can do the job of the command, let the shell handle it.
- Functions should be reused as much as possible.
- Binaries need to be instantly obvious on their purpose.

## Directories and files
- bin/ created only by to be added build script.
- build.sh will compile and install binaries with high interactivity.
- src/* contains code files and a to be added common header.
- README.md contains basic project info and objectives.

## AI agent behaviour rules
- NEVER git push. The user will always do it themselves.
- Always fix a dirty repo if the changes were done by you alone.
- Commits should only be done when the user asks.
- Don't make multiple small commits.
- Testing should only be done when the user asks. Always use timeout to prevent hangs.
- You're here to only assist the user, not write code for them. If the user has trouble, you should only lightly help them.
- Code changes involving the basics fundamentals of code should be avoided, unless the user clearly knows what they are doing.

## Current implementations
### scan
Replacement for ls and find.

### Planed implementantions are in the README.
