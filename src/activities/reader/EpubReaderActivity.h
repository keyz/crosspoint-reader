#pragma once
#include <Epub.h>
#include <Epub/FootnoteEntry.h>
#include <Epub/Section.h>

#include <optional>

#include "BookmarkEntry.h"
#include "EpubReaderMenuActivity.h"
#include "ProgressMapper.h"
#include "activities/Activity.h"

class EpubReaderActivity final : public Activity {
  std::shared_ptr<Epub> epub;
  std::unique_ptr<Section> section = nullptr;
  int currentSpineIndex = 0;
  int nextPageNumber = 0;
  std::optional<uint16_t> pendingPageJump;
  // Set when navigating to a footnote href with a fragment (e.g. #note1).
  // Cleared on the next render after the new section loads and resolves it to a page.
  std::string pendingAnchor;
  int pagesUntilFullRefresh = 0;
  int cachedSpineIndex = 0;
  int cachedChapterTotalPageCount = 0;
  unsigned long lastPageTurnTime = 0UL;
  unsigned long pageTurnDuration = 0UL;
  // millis() at the page-turn input event; consumed (and zeroed) by the next
  // display step to log the button-to-visible latency.
  unsigned long turnRequestedAt = 0UL;
  // Signals that the next render should reposition within the newly loaded section
  // based on a cross-book percentage jump.
  bool pendingPercentJump = false;
  // Normalized 0.0-1.0 progress within the target spine item, computed from book percentage.
  float pendingSpineProgress = 0.0f;
  bool pendingScreenshot = false;
  bool pendingSyncSaveError = false;
  bool skipNextButtonCheck = false;  // Skip button processing for one frame after subactivity exit
  bool automaticPageTurnActive = false;
  bool showBookmarkMessage = false;
  bool ignoreNextConfirmRelease = false;
  bool currentPageBookmarked = false;
  bool bookmarkRemoved = false;  // true when last toggle removed (controls popup text)
  std::vector<BookmarkEntry> cachedBookmarks;
  // Tracks whether this book is currently removed from Recent Books by the
  // removeReadBooksFromRecents feature (set at End-of-Book, cleared if paged back in).
  bool recentsEntryRemoved = false;
  unsigned long bookmarkMessageTime = 0UL;
  // Set when the reader is left at end-of-book and SETTINGS.moveFinishedToReadFolder is on.
  // Consumed in onExit() to relocate the finished book into /Read/.
  bool pendingReadFolderMove = false;

  // Progress-save debounce: progress.bin is written every Nth page turn or on
  // a chapter change instead of on every turn (SD sectors have a finite erase
  // cycle limit), and flushed unconditionally in onExit().
  static constexpr uint8_t PROGRESS_SAVE_INTERVAL = 5;
  uint8_t turnsSinceProgressSave = 0;
  int lastSavedSpineIndex = -1;

  // Speculative next-page pre-render: after a page is displayed, the NEXT page
  // is drawn into the framebuffer during idle time so a forward turn only pays
  // the e-ink refresh. While parked, the framebuffer no longer matches the
  // screen; frameIsSpeculative guards the few paths that draw over the
  // framebuffer assuming it mirrors the panel. The {spine, page} token is
  // single-shot: the next render() either consumes it or discards it.
  bool speculationValid = false;
  bool frameIsSpeculative = false;
  // Skip speculation while the user pages backward — the forward guess would
  // be discarded every turn and its cost would delay each backward render.
  bool lastTurnWasBackward = false;
  int specSpineIndex = -1;
  int specPageNumber = -1;
  // Footnotes captured from the speculative page, moved into
  // currentPageFootnotes at consume time (bounded by MAX_FOOTNOTES_PER_PAGE).
  std::vector<FootnoteEntry> speculativeFootnotes;

  // Footnote support
  std::vector<FootnoteEntry> currentPageFootnotes;
  struct SavedPosition {
    int spineIndex;
    int pageNumber;
  };
  static constexpr int MAX_FOOTNOTE_DEPTH = 3;
  SavedPosition savedPositions[MAX_FOOTNOTE_DEPTH] = {};
  int footnoteDepth = 0;

  void renderContents(std::unique_ptr<Page> page, int orientedMarginTop, int orientedMarginRight,
                      int orientedMarginBottom, int orientedMarginLeft);
  void renderGrayscalePasses(const Page& page, int fontId, int marginLeft, int marginTop, bool pageHasImages);
  void computeContentMargins(int& top, int& right, int& bottom, int& left) const;
  void speculativeRenderNextPage(int orientedMarginLeft, int orientedMarginTop);
  void consumeSpeculativeFrame(int orientedMarginTop, int orientedMarginRight, int orientedMarginBottom,
                               int orientedMarginLeft);
  void restoreFramebufferAfterSpeculation();
  void renderStatusBar() const;
  void silentIndexNextChapterIfNeeded(uint16_t viewportWidth, uint16_t viewportHeight);
  bool saveProgress(int spineIndex, int currentPage, int pageCount);
  void maybeSaveProgress();
  void showPendingSyncSaveErrorPopup();
  void postDisplayTail(int orientedMarginTop, int orientedMarginRight, int orientedMarginBottom,
                       int orientedMarginLeft);
  // Jump to a percentage of the book (0-100), mapping it to spine and page.
  void jumpToPercent(int percent);
  void onReaderMenuConfirm(EpubReaderMenuActivity::MenuAction action);
  // Returns true if sync acted (launched, or surfaced a save error); false if it was a no-op
  // because no KOReader credentials are stored.
  bool launchKOReaderSync();
  void applyOrientation(uint8_t orientation);
  void toggleAutoPageTurn(uint8_t selectedPageTurnOption);
  void pageTurn(bool isForwardTurn);
  void loadCachedBookmarks();
  void addBookmark();
  void updateBookmarkFlag();

  // Footnote navigation
  void navigateToHref(const std::string& href, bool savePosition = false);
  void restoreSavedPosition();

 public:
  explicit EpubReaderActivity(GfxRenderer& renderer, MappedInputManager& mappedInput, std::unique_ptr<Epub> epub)
      : Activity("EpubReader", renderer, mappedInput), epub(std::move(epub)) {}
  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&& lock) override;
  void ensureFramebufferMatchesPanel() override { restoreFramebufferAfterSpeculation(); }
  bool isReaderActivity() const override { return true; }
  ScreenshotInfo getScreenshotInfo() const override;
  CrossPointPosition getCurrentPosition() const;
};
