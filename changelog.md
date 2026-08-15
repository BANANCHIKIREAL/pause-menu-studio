# <c-C6F6FF>v</c><c-96FAEF>4</c><c-72F6D4>.</c><c-7ACECF>6</c><c-A19AED>.</c><c-D06CFF>0</c>

- <c-C6F6FF>Added Global Demonlist placement to the Difficulty card through Global Demonlist Rank's public API.</c>
- <c-ACF8F3>Shows AREDL and Global placements together in one fitted row, with gold and cyan source colors.</c>
- <c-91F9E4>Reads cached Global placements and listens for live API updates without duplicating the Global Demonlist network request.</c>
- <c-7CEFD9>Keeps Global Demonlist Rank fully optional; the existing AREDL/Pemonlist integration continues to work without it.</c>

# <c-C6F6FF>v</c><c-96FAEF>4</c><c-72F6D4>.</c><c-7ACECF>5</c><c-A19AED>.</c><c-D06CFF>4</c>

- <c-C6F6FF>Removed the visible dark rectangle behind the live Globed player count in the Difficulty card.</c>
- <c-ACF8F3>Kept the cyan player total, Globed person icon, half-second refresh, and update animation unchanged.</c>

# <c-C6F6FF>v</c><c-96FAEF>4</c><c-72F6D4>.</c><c-7ACECF>5</c><c-A19AED>.</c><c-D06CFF>3</c>

- <c-C6F6FF>Rebuilt the GitHub README with a custom cyan-to-mint-to-purple hero, glowing gradient dividers, and consistent visual accents across every major section.</c>
- <c-ACF8F3>Reworked the in-game About page with true multi-color headings and gradient emphasis for important controls, modes, and compatibility labels.</c>
- <c-91F9E4>Restyled the complete in-game changelog with gradient version titles and a soft readable color flow across release notes.</c>
- <c-7CEFD9>Kept long descriptions bright and readable instead of applying low-contrast colors to every paragraph.</c>

# <c-C6F6FF>v</c><c-96FAEF>4</c><c-72F6D4>.</c><c-7ACECF>5</c><c-A19AED>.</c><c-D06CFF>2</c>

- <c-C6F6FF>Fixed the editor toolbar showing the obsolete hardcoded version `v4.4.1`.</c>
- <c-ACF8F3>The toolbar now reads the installed version directly from the mod metadata, so future updates cannot leave this label behind.</c>

# <c-C6F6FF>v</c><c-96FAEF>4</c><c-72F6D4>.</c><c-7ACECF>5</c><c-A19AED>.</c><c-D06CFF>1</c>

- <c-C6F6FF>Added a smooth cyan-to-mint-to-purple title gradient to the in-game Geode description.</c>
- <c-ACF8F3>Removed Developer Badges as a dependency because Pause Menu Studio does not use its API.</c>
- <c-91F9E4>Made NONGD / Jukebox optional; without it, the Music card uses Geometry Dash's standard song widget.</c>
- <c-7CEFD9>Updated the README and in-game compatibility notes to state that Pause Menu Studio has no required third-party mods.</c>

# <c-C6F6FF>v</c><c-96FAEF>4</c><c-72F6D4>.</c><c-7ACECF>5</c><c-A19AED>.</c><c-D06CFF>0</c>

- <c-C6F6FF>Added an optional live Globed player-count indicator to the Difficulty card.</c>
- <c-ACF8F3>The count uses Globed's supported soft-link API, includes the local player, and refreshes every half second while the Globed level session is active.</c>
- <c-91F9E4>Added the **Globed player count** setting, enabled by default and available only when the Difficulty card is enabled.</c>
- <c-7CEFD9>Kept Globed optional: the indicator stays hidden instead of showing an incorrect zero when Globed is unavailable, outdated, disconnected, or inactive.</c>

# <c-C6F6FF>v</c><c-96FAEF>4</c><c-72F6D4>.</c><c-7ACECF>4</c><c-A19AED>.</c><c-D06CFF>2</c>

- <c-C6F6FF>Restored the original compact Rename, Copy, and Delete button texture instead of the incompatible panel sprite that rendered as detached colored strips without a texture pack.</c>
- <c-ACF8F3>Rounded only the outer corners of those original action buttons, preserving their previous colors, proportions, icons, and spacing.</c>
- <c-91F9E4>Made the Saved Layouts popup height adapt to one, two, or three visible profile rows instead of reserving a fixed three-row area and leaving most of the window empty.</c>

# <c-C6F6FF>v</c><c-96FAEF>4</c><c-72F6D4>.</c><c-7ACECF>4</c><c-A19AED>.</c><c-D06CFF>1</c>

- <c-C6F6FF>Rebuilt the updater changelog as compact rounded cards with a short colored title, white description, stable icon, and grouped word wrapping.</c>
- <c-ACF8F3>Fixed Shift+Esc level exit with Better Escape when the custom Jukebox song card was active by detaching that card from music-state callbacks before Geometry Dash resets FMOD.</c>
- <c-91F9E4>Fixed custom 512x512 close icons being restored to scale 1 by Geode's Popup API and covering most of the screen.</c>
- <c-7CEFD9>Rebuilt Apply Layout and Update Layout as two distinct folder actions with clean transparent gaps around their inner arrow symbols.</c>
- <c-84D7EA>Rounded the Rename, Copy, and Delete action tiles in saved-layout cards and reduced the Rename pencil size for consistent padding.</c>

# <c-C6F6FF>v</c><c-96FAEF>4</c><c-72F6D4>.</c><c-7ACECF>4</c><c-A19AED>.</c><c-D06CFF>0</c>

- <c-C6F6FF>Added a complete white-on-transparent action icon suite for the editor and its popups.</c>
- <c-ACF8F3>Replaced the Geometry Dash Edit, Done, Advanced Transform, Hide, pagination, layout-management, Trash restore, and popup close icons.</c>
- <c-91F9E4>Added custom Rename, Copy, Restore, Close, navigation, Transform, Generic Block, Apply Layout, and Update Layout resources.</c>
- <c-7CEFD9>Reused the exact Layouts folder contour for Apply Layout and Update Layout; their inner marks sit in a deliberate transparent contour gap like the Save icon.</c>
- <c-84D7EA>Trash cards now use one stable Generic Block symbol instead of random sprites captured from Geometry Dash or another installed mod.</c>
- <c-91BFF3>Added shared icon and labeled-button builders in separate source files instead of adding more UI construction code to `main.cpp`.</c>

# <c-C6F6FF>v</c><c-96FAEF>4</c><c-72F6D4>.</c><c-7ACECF>3</c><c-A19AED>.</c><c-D06CFF>2</c>

- <c-C6F6FF>Added a short original C5-E5 two-note chime for the loading-screen update notification.</c>
- <c-ACF8F3>Bundled the sound as a cross-platform OGG resource instead of relying on a Geometry Dash or texture-pack sound.</c>
- <c-91F9E4>The chime plays at most once per game launch and only after a real available-update banner is shown.</c>
- <c-7CEFD9>Added the **Update notification sound** setting, enabled by default and available independently of update badges.</c>

# <c-C6F6FF>v</c><c-96FAEF>4</c><c-72F6D4>.</c><c-7ACECF>3</c><c-A19AED>.</c><c-D06CFF>1</c>

- <c-C6F6FF>Corrected the misunderstood bottom-toolbar feature from v4.3.0.</c>
- <c-ACF8F3>Removed selection, movement, scaling, advanced transforms, persistence, and Trash handling from the editor toolbar itself.</c>
- <c-91F9E4>Changed the toolbar eye button to collapse only the bottom toolbar while keeping Edit Mode active.</c>
- <c-7CEFD9>Kept normal pause-menu block selection, MOVE, scaling, advanced transforms, Hide, Reset, keyboard movement, Undo, and Redo available while the toolbar is collapsed.</c>
- <c-84D7EA>Added a small eye button at the lower-right edge to restore the toolbar without clearing the selected block.</c>
- <c-91BFF3>The toolbar is shown normally each time Edit Mode is entered.</c>

# <c-C6F6FF>v</c><c-96FAEF>4</c><c-72F6D4>.</c><c-7ACECF>3</c><c-A19AED>.</c><c-D06CFF>0</c>

- <c-C6F6FF>Made the complete bottom editor toolbar selectable through its empty background.</c>
- <c-ACF8F3>Added full toolbar editing through the existing MOVE, exact scale, advanced transform, Reset, arrow-key, and Hide controls.</c>
- <c-91F9E4>Saved the toolbar position, scale, rotation, opacity, drawing layer, and hidden state independently for each menu style and separated gameplay mode.</c>
- <c-7CEFD9>Added a compact **SHOW TOOLBAR** recovery handle outside the hidden panel so editor controls cannot be lost on desktop or Android.</c>
- <c-84D7EA>Updated Edit Mode entrance and exit animations to preserve and animate from the toolbar's custom saved position.</c>
- <c-91BFF3>Full layout Reset now restores and unhides the toolbar together with the other persistent editor controls.</c>

# <c-C6F6FF>v</c><c-96FAEF>4</c><c-72F6D4>.</c><c-7ACECF>2</c><c-A19AED>.</c><c-D06CFF>0</c>

- <c-C6F6FF>Rebuilt the Save Layout window with separate **Save New** and **Update Existing** sections.</c>
- <c-ACF8F3>Existing layouts from the current Normal, Platformer, Creator, or Platformer Creator mode can now be selected and updated directly without retyping their names.</c>
- <c-91F9E4>Added a confirmation step before replacing an existing layout.</c>
- <c-7CEFD9>Updating a layout captures the current positions, scale, rotation, opacity, drawing layer, information cards, and complete Trash state.</c>
- <c-84D7EA>Kept the new-name input unfocused by default so the Android keyboard does not cover the existing-layout list.</c>

# <c-C6F6FF>v</c><c-96FAEF>4</c><c-72F6D4>.</c><c-7ACECF>1</c><c-A19AED>.</c><c-D06CFF>7</c>

- <c-C6F6FF>Fixed the update-details window incorrectly reporting that release notes were missing even when `update.json` contained them.</c>
- <c-ACF8F3>Release notes are now copied before the update candidate is moved into the download callback, removing the argument-evaluation-order bug.</c>

# <c-C6F6FF>v</c><c-96FAEF>4</c><c-72F6D4>.</c><c-7ACECF>1</c><c-A19AED>.</c><c-D06CFF>6</c>

- <c-C6F6FF>Replaced the Edit Mode **Save**, **Layouts**, **Trash**, **Undo**, and **Redo** graphics with a matching pure-white icon set.</c>
- <c-ACF8F3>Save now uses a folder with a downward arrow; Layouts uses a clean open-folder symbol; Trash uses the requested solid bin design.</c>
- <c-91F9E4>Undo and Redo are exact mirrored variants of the same curved arrow, keeping their weight, padding, and scale consistent.</c>
- <c-7CEFD9>Normalized all five icons to transparent `128x128` PNG resources and equalized their visible size in the toolbar.</c>

# <c-C6F6FF>v</c><c-96FAEF>4</c><c-72F6D4>.</c><c-7ACECF>1</c><c-A19AED>.</c><c-D06CFF>5</c>

- <c-C6F6FF>Fixed update checks failing with `HTTP 403` after GitHub's unauthenticated REST API allowance for the current public IP reached `60/60` requests.</c>
- <c-ACF8F3>Replaced GitHub REST API release discovery with the first-party `https://pause-menu-studio.vercel.app/update.json` manifest.</c>
- <c-91F9E4>The updater now validates the manifest version, official GitHub release URL, and required SHA-256 digest before showing or installing an update.</c>
- <c-7CEFD9>GitHub remains the package download host, but checking for an update no longer consumes its REST API rate limit.</c>

# <c-C6F6FF>v</c><c-96FAEF>4</c><c-72F6D4>.</c><c-7ACECF>1</c><c-A19AED>.</c><c-D06CFF>4</c>

- <c-C6F6FF>Added a dedicated update-details window to the in-game updater.</c>
- <c-ACF8F3>Update checks now read the selected GitHub Release's `body` field and show it under **WHAT'S NEW & FIXED** before downloading.</c>
- <c-91F9E4>Added a scrollable release-notes panel so longer fix lists remain readable without covering the action buttons.</c>
- <c-7CEFD9>Converted common Markdown headings, bullets, links, emphasis, and inline code into game-safe text.</c>
- <c-84D7EA>Kept the existing explicit **LATER** and **DOWNLOAD** choices; viewing release notes never starts a download by itself.</c>

# <c-C6F6FF>v</c><c-96FAEF>4</c><c-72F6D4>.</c><c-7ACECF>1</c><c-A19AED>.</c><c-D06CFF>3</c>

- <c-C6F6FF>Disabled Edit Mode keyboard shortcuts while a text input or modal popup is active.</c>
- <c-ACF8F3>Typing layout names can no longer trigger Reset with `R`, remove blocks with `Delete`, move blocks with arrow keys, or run Undo/Redo with `Ctrl+Z` and `Ctrl+Y`.</c>
- <c-91F9E4>Shortcuts resume automatically after the input dialog closes.</c>
- <c-7CEFD9>Kept `CCMenuItemSpriteExtra` press-animation base scales synchronized with editor scaling, so clicking a resized button no longer restores its previous size.</c>

# <c-C6F6FF>v</c><c-96FAEF>4</c><c-72F6D4>.</c><c-7ACECF>1</c><c-A19AED>.</c><c-D06CFF>2</c>

- <c-C6F6FF>Removed the texture-backed square behind update exclamation marks.</c>
- <c-ACF8F3>Changed the update mark itself from white to bright red.</c>
- <c-91F9E4>Attached the pause-menu update mark directly to the upper-right edge of the Edit button so it follows the button's saved position.</c>

# <c-C6F6FF>v</c><c-96FAEF>4</c><c-72F6D4>.</c><c-7ACECF>1</c><c-A19AED>.</c><c-D06CFF>1</c>

- <c-C6F6FF>Replaced the large static loading-screen update banner with a compact top toast.</c>
- <c-ACF8F3>The toast slides down from above the screen, stays visible briefly, and smoothly returns upward without covering the loading status area.</c>
- <c-91F9E4>Added a compact white download icon, rounded dark panel, cyan accent, and duplicate-display protection.</c>

# <c-C6F6FF>v</c><c-96FAEF>4</c><c-72F6D4>.</c><c-7ACECF>1</c><c-A19AED>.</c><c-D06CFF>0</c>

- <c-C6F6FF>Added the **Separate layouts by mode** setting. When enabled, normal, platformer, creator, and creator-platformer pause menus keep independent positions, active named layouts, generations, and Hidden Blocks; disabling it makes every mode use the normal layout namespace without deleting separated data.</c>
- <c-ACF8F3>Added an Advanced Transform popup for exact rotation, opacity, and drawing-layer values. Supported values participate in Undo/Redo, per-block Reset, full Reset, persistent transforms, named layouts, Apply, Trash, and Restore.</c>
- <c-91F9E4>Added independent settings for automatic startup checks, the loading-screen update banner, and red update badges. Manual checks from Edit Mode remain available when automatic checking is disabled.</c>
- <c-7CEFD9>Added a configurable Undo/Redo history limit from 10 to 200 actions, with a default of 50.</c>
- <c-84D7EA>Saving a named layout now commits pending Edit Mode transforms first, so a newly changed block is included without requiring DONE beforehand.</c>

# <c-C6F6FF>v</c><c-96FAEF>4</c><c-72F6D4>.</c><c-7ACECF>0</c><c-A19AED>.</c><c-D06CFF>4</c>

- <c-C6F6FF>Fixed the automatic update check not starting on the first Geode loading screen because Pause Menu Studio was loaded after `LoadingLayer::init` had already run.</c>
- <c-ACF8F3>The update banner now attaches to the currently visible loading screen as soon as Pause Menu Studio loads.</c>
- <c-91F9E4>Added the red update `!` directly above the pause-menu Edit button, while retaining the badge on the Updates toolbar button.</c>
- <c-7CEFD9>Moved the loading banner above Geode's two status labels so the three messages do not overlap.</c>

# <c-C6F6FF>v</c><c-96FAEF>4</c><c-72F6D4>.</c><c-7ACECF>0</c><c-A19AED>.</c><c-D06CFF>3</c>

- <c-C6F6FF>Fixed the stock pause-menu heading of official levels changing to the active Jukebox NONG/remix title after resuming and pausing again.</c>
- <c-ACF8F3>The heading now uses the real level name captured when the PlayLayer starts, without changing Jukebox's selected song or song-card metadata.</c>

# <c-C6F6FF>v</c><c-96FAEF>4</c><c-72F6D4>.</c><c-7ACECF>0</c><c-A19AED>.</c><c-D06CFF>2</c>

- <c-C6F6FF>Added an automatic GitHub Releases check while Geometry Dash is loading.</c>
- <c-ACF8F3>Added a loading-screen banner when a newer Pause Menu Studio release is available.</c>
- <c-91F9E4>Added a persistent red `!` above the Updates icon while an update is available.</c>
- <c-7CEFD9>Increased the size, brightness, and minimum scale of the selected block's scale/movement status text.</c>

# <c-C6F6FF>v</c><c-96FAEF>4</c><c-72F6D4>.</c><c-7ACECF>0</c><c-A19AED>.</c><c-D06CFF>1</c>

- <c-C6F6FF>Replaced the Updates, Layouts, and Move artwork with the supplied clean white icons.</c>
- <c-ACF8F3>Converted every new icon to a true transparent game sprite so no checkerboard or black square is rendered in Geometry Dash.</c>
- <c-91F9E4>Fixed the selected-block inspector divider overlapping the Reset control.</c>

# <c-C6F6FF>v</c><c-96FAEF>4</c><c-72F6D4>.</c><c-7ACECF>0</c><c-A19AED>.</c><c-D06CFF>0</c>

- <c-C6F6FF>Added an Updates button directly to the pause-menu editor.</c>
- <c-ACF8F3>Added an in-game updater backed by the official Pause Menu Studio GitHub Releases feed, including stable and prerelease version comparison.</c>
- <c-91F9E4>Added download progress, confirmation before downloading, SHA-256 verification when GitHub provides a digest, package identity and version validation, and Geometry Dash/Geode compatibility validation.</c>
- <c-7CEFD9>Updates are staged by safely replacing the installed `.geode` package and become active only after the player confirms or manually performs a game restart.</c>
- <c-84D7EA>Added recovery of the original package if replacement fails, preventing a partial download from removing the working installation.</c>
- <c-91BFF3>Expanded and evenly spaced the editor dock for eight controls, with separators centered between groups instead of touching the Reset card.</c>

# <c-C6F6FF>v</c><c-B7F7FB>3</c><c-A7F8F6>.</c><c-82FCE5>3</c><c-75FFDA>.</c><c-72F6D4>0</c><c-73DDCC>-</c><c-7ACECF>b</c><c-84BDD8>e</c><c-91ACE3>t</c><c-A19AED>a</c><c-C276FF>.</c><c-D06CFF>2</c>

- <c-C6F6FF>Replaced texture-pack-sensitive stretched editor tiles with stable Geometry Dash panel textures.</c>
- <c-ACF8F3>Removed the unwanted square background behind the pause-menu Edit button.</c>
- <c-91F9E4>Increased the size, weight, contrast, and readability of toolbar captions, the studio header, version text, and the Scale label.</c>
- <c-7CEFD9>Made the mode badge substantially smaller and removed the unnecessary Ready label.</c>
- <c-84D7EA>Added visible spacing between the Reset and Hide controls in the selected-block inspector.</c>
- <c-91BFF3>Restored editor-panel transparency without fading the icons or labels, and removed the extra dark layer that made the dock look solid black.</c>

# <c-C6F6FF>v</c><c-B7F7FB>3</c><c-A7F8F6>.</c><c-82FCE5>3</c><c-75FFDA>.</c><c-72F6D4>0</c><c-73DDCC>-</c><c-7ACECF>b</c><c-84BDD8>e</c><c-91ACE3>t</c><c-A19AED>a</c><c-C276FF>.</c><c-D06CFF>1</c>

- <c-C6F6FF>Rebuilt Edit Mode as a layered GD-style studio interface with a deeper rounded dock, individual action tiles, grouped controls, mode accents, and a visible Beta 1 identity.</c>
- <c-ACF8F3>Reworked the selection inspector with a compact header, live scale and movement status, clearer action grouping, animated appearance, and mode-colored highlights.</c>
- <c-91F9E4>Added a compact mode badge with creator/platformer colors, an unsaved-state color, and smooth entrance and exit transitions.</c>
- <c-7CEFD9>Added staggered dock-button entrance animation, refined panel pulses, improved disabled Undo/Redo states, and a redesigned Back to Edit control for Preview Mode.</c>

# <c-C6F6FF>v</c><c-96FAEF>3</c><c-72F6D4>.</c><c-7ACECF>2</c><c-A19AED>.</c><c-D06CFF>1</c>

- <c-C6F6FF>Added Creator Edit Mode for levels launched from the Geometry Dash level editor.</c>
- <c-ACF8F3>Added full editor support for the level-editor pause button: move, resize, reset, Hide/Trash, restore, and named layouts.</c>
- <c-91F9E4>Separated creator and platformer-creator positions, active layouts, layout generations, and Hidden Blocks from normal and platformer menus.</c>

# <c-C6F6FF>v</c><c-96FAEF>3</c><c-72F6D4>.</c><c-7ACECF>2</c><c-A19AED>.</c><c-D06CFF>0</c>

- <c-C6F6FF>Updated the required Geode version from 5.8.2 to 5.9.0 and rebuilt all supported binaries against the new SDK.</c>
- <c-ACF8F3>Migrated Pause Menu Studio's editor, Saved Layouts, and Hidden Blocks labels to the new `geode::Label` API for automatic size limits and more efficient text updates.</c>
- <c-91F9E4>Shrunk the selection contextual panel (width 420 -> 400, height 78 -> 68) and tightened its internal spacing so it covers less of the screen.</c>
- <c-7CEFD9>Added automatic Platformer Edit Mode, including readable targets for the platformer play-time display and additional pause buttons.</c>
- <c-84D7EA>Added independent platformer position keys and layout generations without changing existing normal-mode position keys.</c>
- <c-91BFF3>Separated normal and platformer named-layout lists, active layouts, and Hidden Blocks storage so mode-specific controls cannot leak between menus.</c>

# <c-C6F6FF>v</c><c-96FAEF>3</c><c-72F6D4>.</c><c-7ACECF>1</c><c-A19AED>.</c><c-D06CFF>0</c>

- <c-C6F6FF>Removed saved-layout thumbnail previews completely: no guessed icons, dots, or empty preview box.</c>
- <c-ACF8F3>Expanded the layout-name area into the freed space for a cleaner Saved Layouts card.</c>

# <c-C6F6FF>v</c><c-96FAEF>3</c><c-72F6D4>.</c><c-7ACECF>0</c><c-A19AED>.</c><c-D06CFF>3</c>

- <c-C6F6FF>Saved-layout previews now store each block's normalized world-space center instead of its parent-local coordinates, preventing icons from piling up in the lower-left corner on any aspect ratio.</c>
- <c-ACF8F3>Legacy layouts without preview coordinates no longer draw misleading piled-up icons; resaving them creates a correct preview.</c>
- <c-91F9E4>Hidden Blocks now renders four entries per page with page controls, preventing a large Trash from producing an empty or overloaded popup.</c>

# <c-C6F6FF>v</c><c-96FAEF>3</c><c-72F6D4>.</c><c-7ACECF>0</c><c-A19AED>.</c><c-D06CFF>2</c>

- <c-C6F6FF>Delete is now the only keyboard shortcut that moves a selected block to Trash; Backspace no longer removes blocks.</c>
- <c-ACF8F3>The numeric scale field no longer rewrites its formatted value while focused, so Backspace can edit it normally.</c>
- <c-91F9E4>FREE MOVE now remembers its enabled or disabled state separately for each selected block while the editor is open.</c>
- <c-7CEFD9>Replaced both RESET controls and VIEW with custom white reset and eye artwork.</c>
- <c-84D7EA>Removed the orange 1.00 line and label from the scale slider.</c>
- <c-91BFF3>Trash now captures the actual cached sprite frame from each removed object instead of choosing an icon from its name; objects without a reusable frame get one neutral info icon.</c>
- <c-A19AED>Saved-layout previews now store actual cached object sprite frames; old or unavailable frames use neutral dots instead of guessed icons.</c>
- <c-B287F7>Re-captures icons for existing Trash entries when Hidden Blocks opens.</c>
- <c-C276FF>Added a larger gap between REN and COPY and moved the layout delete button away from them.</c>

# <c-C6F6FF>v</c><c-96FAEF>3</c><c-72F6D4>.</c><c-7ACECF>0</c><c-A19AED>.</c><c-D06CFF>1</c>

- <c-C6F6FF>Added a real MOVE lock: selecting a block no longer moves it until MOVE is enabled.</c>
- <c-ACF8F3>Added direct numeric scale input.</c>
- <c-91F9E4>Increased spacing between toolbar and saved-layout actions.</c>
- <c-7CEFD9>Added a smooth exit animation when leaving Edit Mode.</c>
- <c-84D7EA>Replaced RESET and VIEW with dedicated Geometry Dash icons.</c>
- <c-91BFF3>Saved the exact Trash contents and their icon types inside every named layout.</c>
- <c-A19AED>Replaced dot-only layout previews with miniature Geometry Dash block icons.</c>
- <c-B287F7>Added rounded Geometry Dash frames to editor panels, layout cards, previews, and Trash cards.</c>
- <c-C276FF>Improved native icon matching for settings, music, SFX, comments, coins, demons, ratings, levels, and other hidden blocks.</c>

# <c-C6F6FF>v</c><c-96FAEF>3</c><c-72F6D4>.</c><c-7ACECF>0</c><c-A19AED>.</c><c-D06CFF>0</c>

- <c-C6F6FF>Rebuilt Edit Mode around a single dark bottom toolbar with large touch targets for Undo, Redo, Save, Layouts, Hidden Blocks, Reset, and Preview.</c>
- <c-ACF8F3>Added a contextual selection panel that follows the selected logical block and provides Move guidance, a live scale slider, local scale reset, and Hide.</c>
- <c-91F9E4>Added Preview Mode, which hides the editor chrome for a clean, read-only view until Return is pressed.</c>
- <c-7CEFD9>Added animated editor entry, selection-panel appearance, undo/redo feedback, trash feedback, and popup-card entry.</c>
- <c-84D7EA>Added focused selection visuals with thin cyan or orange outlines, corner handles, a logical-block name, and safe dimming around—not over—the selection.</c>
- <c-91BFF3>Redesigned Saved Layouts as cards with miniature position previews, saved dates, Apply, Rename, Duplicate, and Delete actions.</c>
- <c-A19AED>Redesigned Hidden Blocks as a two-column visual grid with native Geometry Dash icons and large Restore buttons.</c>
- <c-B287F7>Added a touch-friendly scale slider and enlarged editor hit areas for Android while keeping physical-keyboard shortcuts available.</c>
- <c-C276FF>Kept existing named-layout snapshots and Hidden Blocks storage compatible; older layouts appear as Legacy Layout until saved again.</c>

# <c-C6F6FF>v</c><c-96FAEF>2</c><c-72F6D4>.</c><c-7ACECF>2</c><c-A19AED>.</c><c-D06CFF>1</c>

- <c-C6F6FF>Added the official Pause Menu Studio website to the Geode mod card.</c>
- <c-ACF8F3>Added BANANCHIKIREAL Badges API v1.2.4 as a required dependency for the developer badge.</c>

# <c-C6F6FF>v</c><c-96FAEF>2</c><c-72F6D4>.</c><c-7ACECF>2</c><c-A19AED>.</c><c-D06CFF>0</c>

- <c-C6F6FF>Added official Android support for Geometry Dash 2.2081 on both Android32 and Android64.</c>
- <c-ACF8F3>Added a multi-platform GitHub Actions workflow that builds Windows, Android32, and Android64 packages and combines their artifacts.</c>
- <c-91F9E4>Documented Android installation, touch controls, architecture selection, and Android build commands.</c>

# <c-C6F6FF>v</c><c-96FAEF>2</c><c-72F6D4>.</c><c-7ACECF>1</c><c-A19AED>.</c><c-D06CFF>0</c>

- <c-C6F6FF>Level cards now keep the original level name captured when PlayLayer starts, so reopening the pause menu cannot replace an official level title with Jukebox's active NONG/cover title.</c>
- <c-ACF8F3>Coins and Difficulty information cards are now disabled by default on new configurations.</c>
- <c-91F9E4>Named layouts now record which information cards exist, recreate theme cards even when their normal settings are disabled, and keep those cards across pause-menu reopenings.</c>
- <c-7CEFD9>Full Reset now removes theme-only cards and returns card visibility to the normal mod settings.</c>

# <c-C6F6FF>v</c><c-96FAEF>2</c><c-72F6D4>.</c><c-7ACECF>0</c><c-A19AED>.</c><c-D06CFF>7</c>

- <c-C6F6FF>Hidden Blocks now audits the active layout and currently invisible Pause Menu Studio controls, returning missing managed blocks to Trash before the list opens.</c>
- <c-ACF8F3>Applying a named layout no longer deletes Trash entries whose dynamic mod nodes are temporarily unavailable.</c>
- <c-91F9E4>Named layouts now store every Trash member, including hidden dynamic controls that are absent while the layout is saved.</c>

# <c-C6F6FF>v</c><c-96FAEF>2</c><c-72F6D4>.</c><c-7ACECF>0</c><c-A19AED>.</c><c-D06CFF>6</c>

- <c-C6F6FF>Fixed the PauseLayer retain cycle reintroduced by the one-second entrance-layout guard; pressing Space immediately after Esc now lets the game remove the pause menu normally.</c>
- <c-ACF8F3>Position and scale edits are now staged in Edit Mode and written only when Done is pressed; leaving the level through the unsaved-changes warning discards those temporary transforms.</c>
- <c-91F9E4>Fixed Jukebox's disc being pinned by its title-dependent nong-menu origin; placement now uses the visible nong-pin center, keeping it on the music panel's top-left corner for every song title.</c>
- <c-7CEFD9>Removed the one-second layout jump on pause-menu entry by reapplying saved geometry after each entrance-animation frame instead of waiting for one delayed correction.</c>
- <c-84D7EA>Fixed the confirmation popup's Exit button leaving an orphan pause layer behind: confirmed exits now use PauseLayer's complete quit path, which closes both the level and its pause UI.</c>

# <c-C6F6FF>v</c><c-96FAEF>2</c><c-72F6D4>.</c><c-7ACECF>0</c><c-A19AED>.</c><c-D06CFF>3</c>

- <c-C6F6FF>Fixed Apply resetting resized controls to their default scale when a dynamic mod node no longer matched the saved profile path.</c>
- <c-ACF8F3>Named layouts now save and restore their Trash state, including grouped hidden controls; older layouts preserve the current Trash when applied.</c>
- <c-91F9E4>Replaced the default-size control icon with Geometry Dash's native editor reset icon.</c>

# <c-C6F6FF>v</c><c-96FAEF>2</c><c-72F6D4>.</c><c-7ACECF>0</c><c-A19AED>.</c><c-D06CFF>0</c>

- <c-C6F6FF>Fixed hidden blocks occasionally disappearing without a Trash entry when two sanitized node paths produced the same storage ID; Trash entries are now unique and verified before a block is hidden.</c>
- <c-ACF8F3>Replaced the layout editor's text controls with native Geometry Dash icon buttons that also follow installed texture packs.</c>
- <c-91F9E4>Changed the default menu style from Showcase to Classic.</c>
- <c-7CEFD9>Fixed the confirmed Exit action failing to leave the level when Better Escape is installed.</c>
- <c-84D7EA>Fixed tiny selection outlines for music and fire widgets by measuring their visible descendants instead of technical container sizes.</c>
- <c-91BFF3>Multi-selection created with Ctrl+click now uses an orange outline; single selection remains cyan.</c>
- <c-A19AED>Stabilized the Jukebox music block after repeated pause-menu openings so its title no longer drifts onto the level-name card.</c>
- <c-B287F7>Re-pins Jukebox's recreated disc for several frames after a song or multi-asset change, so different title lengths cannot move it.</c>
- <c-C276FF>Repairs Better Volume slider-thumb geometry after menu-animation actions without disabling the texture pack's sprite animations.</c>
- <c-D98AFF>Repeated clicks at the same cursor position now cycle through overlapping logical blocks from front to back.</c>

# <c-C6F6FF>v</c><c-96FAEF>1</c><c-72F6D4>.</c><c-7ACECF>5</c><c-A19AED>.</c><c-D06CFF>1</c>

- <c-C6F6FF>Fixed Jukebox's disc moving between NCS and regular songs by pinning it to the music block's top-left corner after every song-info rebuild.</c>
- <c-ACF8F3>Stopped old saved child offsets from being restored inside music, level, coin, and difficulty cards.</c>
- <c-91F9E4>Added an exit warning when Edit Mode is still active and the current pause-menu layout was changed.</c>
- <c-7CEFD9>Added Better Escape compatibility: normal Esc still resumes, while Shift+Esc and the quit button show the edit warning before leaving the level.</c>
- <c-84D7EA>Full RESET now restores every hidden block to its default position, makes it visible, and clears the trash.</c>
- <c-91BFF3>Better Volume groups now receive one shared screen-edge correction, preventing their parts from compressing at the top or sides.</c>
- <c-A19AED>Enlarged the mod-list logo so the emblem fills the available icon area.</c>

# <c-C6F6FF>v</c><c-96FAEF>1</c><c-72F6D4>.</c><c-7ACECF>5</c><c-A19AED>.</c><c-D06CFF>0</c>

- <c-C6F6FF>Added per-block reset with confirmation. Select a block and press R, or use RESET while a block is selected.</c>
- <c-ACF8F3>Added HIDE plus Delete/Backspace shortcuts to move selected logical blocks into a persistent trash.</c>
- <c-91F9E4>Added a trash popup that restores hidden blocks to their exact position and size from before removal.</c>
- <c-7CEFD9>Better Volume Music and SFX groups are hidden, restored, and reset as complete logical blocks.</c>
- <c-84D7EA>Replaced the mod logo with a new cyan-and-gold pause-menu editor emblem with transparent outer corners.</c>

# <c-C6F6FF>v</c><c-96FAEF>1</c><c-72F6D4>.</c><c-7ACECF>4</c><c-A19AED>.</c><c-D06CFF>1</c>

- <c-C6F6FF>Fixed Better Volume's Music and SFX selection overlay: each section now appears as one logical block with one outline around its visible controls instead of separate or screen-sized technical bounds.</c>

# <c-C6F6FF>v</c><c-96FAEF>1</c><c-72F6D4>.</c><c-7ACECF>4</c><c-A19AED>.</c><c-D06CFF>0</c>

- <c-C6F6FF>A click now selects an element without moving it; dragging starts only after the cursor crosses a movement threshold.</c>
- <c-ACF8F3>Added Ctrl+click multi-selection and synchronized dragging for selected elements.</c>
- <c-91F9E4>Added arrow-key movement (1 unit, or 5 units while holding Shift) with Undo/Redo history.</c>
- <c-7CEFD9>Added Size-, 100%, and Size+ controls with persistent scale and named-layout support.</c>
- <c-84D7EA>Better Volume's music controls and SFX controls are now treated as two complete logical groups.</c>
- <c-91BFF3>Made the Edit/Done button movable while Edit mode is active.</c>
- <c-A19AED>Fixed the Save button escaping its layout-name popup.</c>
- <c-B287F7>Fixed empty space in the cards row selecting and moving level, music, and difficulty cards together.</c>
- <c-C276FF>Level Name and Music cards now default to disabled.</c>
- <c-D98AFF>Fixed official levels displaying an NA/unrated difficulty label when Geometry Dash has a calculated difficulty.</c>

# <c-C6F6FF>v</c><c-96FAEF>1</c><c-72F6D4>.</c><c-7ACECF>3</c><c-A19AED>.</c><c-D06CFF>0</c>

- <c-C6F6FF>Added named pause-menu layouts that can be saved, selected, applied, overwritten, and deleted from Edit mode.</c>
- <c-ACF8F3>Moved named-layout storage and popup UI out of `main.cpp` into dedicated source files.</c>
- <c-91F9E4>Removed the opaque black corners from the mod icon and replaced them with transparency.</c>

# <c-C6F6FF>v</c><c-96FAEF>1</c><c-72F6D4>.</c><c-7ACECF>2</c><c-A19AED>.</c><c-D06CFF>0</c>

- <c-C6F6FF>Fixed RESET corrupting animated texture-pack buttons by restoring only elements actually moved by Pause Menu Studio.</c>
- <c-ACF8F3>Added a confirmation popup before resetting the layout.</c>
- <c-91F9E4>Removed the PauseLayer retain cycle that could leave the pause menu alive after resuming with Space.</c>
- <c-7CEFD9>Replaced per-frame position enforcement with one delayed restore after entrance animations finish.</c>
- <c-84D7EA>Added more spacing between the Undo, Redo, and Reset buttons.</c>

# <c-C6F6FF>v</c><c-96FAEF>1</c><c-72F6D4>.</c><c-7ACECF>1</c><c-A19AED>.</c><c-D06CFF>1</c>

- <c-C6F6FF>Removed the bright editor grid.</c>
- <c-ACF8F3>Added undo and redo history with `Ctrl+Z`, `Ctrl+Y`, and in-editor buttons.</c>
- <c-91F9E4>Replaced the reset setting toggle with a direct `RESET` button in Edit mode.</c>
- <c-7CEFD9>Reapplied saved positions every frame so menu-animation texture packs cannot overwrite custom positions.</c>
- <c-84D7EA>Explicitly updated feature state for compatibility with GodlikeFaces fire and Legendary/Mythic faces.</c>
- <c-91BFF3>Added a rounded Geometry Dash-style frame to the mod icon.</c>

# <c-C6F6FF>v</c><c-96FAEF>1</c><c-72F6D4>.</c><c-7ACECF>1</c><c-A19AED>.</c><c-D06CFF>0</c>

- <c-C6F6FF>Added Geometry Dash's real star sprite beside the star count.</c>
- <c-ACF8F3>Added exact Demons In Between and GDDP difficulty faces when those mods enable them.</c>
- <c-91F9E4>Added asynchronous AREDL/Pemonlist placement from Integrated Demonlist's data sources.</c>
- <c-7CEFD9>Enabled the real multi-asset `CustomSongWidget` mode so songs, SFX, total size, and the info button are shown.</c>
- <c-84D7EA>Music and information cards now move as complete logical groups in Edit mode.</c>
- <c-91BFF3>Moving a difficulty card now keeps its face, feature state, star reward, and Demonlist rank together.</c>
- <c-A19AED>Reset incompatible v1.0.x child positions and compacted the editor status display.</c>

# <c-C6F6FF>v</c><c-96FAEF>1</c><c-72F6D4>.</c><c-7ACECF>0</c><c-A19AED>.</c><c-D06CFF>3</c>

- <c-C6F6FF>Replaced the imitation music panel with the real `CustomSongWidget` used and extended by Jukebox 3.6.2.</c>
- <c-ACF8F3>The music panel now uses Jukebox's real `JB_PinDisc.png` button and native song controls.</c>
- <c-91F9E4>Added compatibility with Gold User Coins 2.0.4 by using its swapped `secretCoinUI_001.png` frame.</c>
- <c-7CEFD9>Replaced the difficulty text with Geometry Dash's real `GJDifficultySprite`.</c>
- <c-84D7EA>Fixed rated levels being shown as Unrated by using `GJGameLevel::getAverageDifficulty()`.</c>
- <c-91BFF3>Reworked Edit mode selection, smooth dragging, screen-edge protection, and selection outlines.</c>
- <c-A19AED>Added a dedicated Pause Menu Studio mod icon.</c>

# <c-C6F6FF>v</c><c-96FAEF>1</c><c-72F6D4>.</c><c-7ACECF>0</c><c-A19AED>.</c><c-D06CFF>0</c>

- <c-C6F6FF>Initial release.</c>
- <c-ACF8F3>Added draggable pause-menu elements with saved positions.</c>
- <c-91F9E4>Added Classic, Compact, and Showcase presets.</c>
- <c-7CEFD9>Added configurable level, music, coin, and difficulty panels.</c>
