# include/StoryRoute.hpp

Из `Next.md` сюда вынесены дополнительные objective helper-ы:

- `CurrentStoryCheckpointLabel(const SessionProfile&)`
- `CurrentStoryObjectivePreview(const SessionProfile&)`
- `BuildBt72RestorationRoute(const SessionProfile&)`
- `HasLanlineServicesObjective(const SessionProfile&)`
- `HasFeyRingIntercityObjective(const SessionProfile&)`
- `HasFeyRingInterserverObjective(const SessionProfile&)`

Это должен быть тонкий слой, который берет истину из `SessionProfile` / `worldFieldStates` и держит launcher/runtime в одной route-фазе без дублирования логики.
