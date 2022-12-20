## Sites
Sites that I open every day.
## Install
Install `dmenu`
## Usage
### Add new site
`sites [-n -s short -u url [-b before_command] [-a after_command]]`
>switch to first tap in DWM then open leetcode in browser, when the tab is closed open google tasks
 `sites -n -s leetcode -u https://leetcode.com -b 'xdotool key super+1' -a '$BROWSER https://calendar.google.com/calendar/u/1/r/day'`

### Modify site
`sites [-m short [-s short] [-u url] [-b before_command] [-a after_command] [-f 0(no)|1(yes)|2(toggle)]]`

### Remove site
`sites [-r short]`

### List sites
`sites [-l [-s] [-u] [-b] [-a] [-f (finished)] [-F (unfinished)]]`
### Open site
`sites`
