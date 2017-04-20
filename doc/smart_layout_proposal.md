# SmartLayoutSprite Proposal / Request for comments

## Concept

I primarily wrote this to make working with xml layouts & setting sprites easier and less error prone.
Essentially it loads an xml layout, and stores the sprite map. The setter
functions all cast as needed, and check that the sprite exists, logging a
warning if it does not.

Instead of / in addition to using the normal event handler function, it also
provides allows registering callbacks for any given event (and as an upgrade,
could also cast the event to the correct type).

The last unique feature is that all of the functions that don't return a value
return a reference to 'this', which means that the function calls can be
chained/grouped logically.

## Mini example

This is taken from [NWM Life Journeys](https://github.com/Downstream/nwm_life_journeys),
where some of the function names are different, and the class is called
ModelSprite rather than SmartLayoutSprite

```c++
auto balanceSprite = new SmartLayoutSprite(mGlobals, "%APP%/data/layouts/component/balance.xml");
balanceSprite->setTextSprite("label", budgetData.getBalanceIndicatorLabel())
    .setSpriteText("good", budgetData.getBudgetBalancedLabel())
    .setSpriteText("okay", budgetData.getBudgetOkLabel())
    .setSpriteText("bad", budgetData.getBudgetUnbalancedLabel())
    .addSpriteChild("label_layout", new TooltipButton(mGlobals, budgetData.getBalanceIndicatorLabel(),  budgetData.getBalanceIndicatorTooltip()))
    .animateOn();
```

## Proposed class

```c++
class SmartLayoutSprite : public ds::ui::Sprite {
  public:
    SmartLayoutSprite(Globals& g, std::string xmlLayoutFile);

    void setup();

    bool hasSprite(std::string spriteName)
    ds::ui::Sprite* getSprite(std::string spriteName);
    ModelSprite& addSpriteChild(std::string spriteName, ds::ui::Sprite* newChild);

    // Text sprite functions
    ModelSprite& setSpriteText(std::string, std::wstring);
    ModelSprite& setSpriteText(std::string, std::string);
    ModelSprite& setSpriteFont(std::string, std::string);

    // Image sprite Functions
    ModelSprite& setSpriteImage(std::string, std::string);
    ModelSprite& setSpriteImage(std::string, ds::Resource);

    // Common sprite function
    ModelSprite& setSpriteOpacity(std::string, float);
    ModelSprite& setSpriteSize(std::string, ci::vec2);
    ModelSprite& setSpriteScale(std::string, ci::vec2);
    ModelSprite& setSpriteColor(std::string, std::string);
    ModelSprite& setSpritePosition(std::string, ci::vec2);

    // Animation
    ModelSprite& tweenSpriteOpacity(std::string spriteName, float value, float duration, float delay, finishFn, updateFn);
    ModelSprite& tweenSpriteScale(std::string spriteName, ci::vec3 value, float duration, float delay, finishFn, updateFn);
    ModelSprite& tweenSpriteSize(std::string spriteName, ci::vec3 value, float duration, float delay, finishFn, updateFn);

    // Callback event handling
    ModelSprite& listen(int type, std::function<void(const ds::Event&)> callback);

    // Touch control
    ModelSprite& setSpriteTapFn(std::string spriteName, const std::function<void(ds::ui::Sprite*, const ci::vec3&)>& tapCallback);
    ModelSprite& enableSprite(std::string spriteName, bool enable);

    ModelSprite& doLayout();

    ModelSprite& animateOn();
    ModelSprite& animateOff();
}
```
