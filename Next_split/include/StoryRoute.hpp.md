# include/StoryRoute.hpp

Из `Next.md` сюда вынесены дополнительные objective helper-ы:

- `HasLanlineServicesObjective(const SessionProfile&)`
- `HasFeyRingIntercityObjective(const SessionProfile&)`
- `HasFeyRingInterserverObjective(const SessionProfile&)`

Это должен быть тонкий слой, который берет истину из `worldFieldStates`, а не придумывает свой отдельный статус.
