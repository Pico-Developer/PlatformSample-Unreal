# Platform SDK Demo

The demo demonstrates how to use PICO Unreal Platform SDK to integrate the Platform services for online games.

For detailed instructions, visit [Demo documentation](https://developer-cn.pico-interactive.com/en/document/unreal/identity-and-friend-demo/). For suggestions and questions, visit [PICO community](https://developer-cn.pico-interactive.com/community/?cate=4).

## Requirements

| Environment          | Version             |
|----------------------|---------------------|
| Unreal Engine        |   4.25 4.26 4.27    |
| PICO Integration SDK | &ge;2.1.4           |
| PICO device system     | &ge;4.6.0    |


Download Unreal Integration SDK from this [link](https://developer-cn.pico-interactive.com/sdk?deviceId=1&platformId=2&itemId=13).

## Description

The platform services are as follows:


| Service              | Description                 |
|---------------------|----------------------|
| [Identity & Friend](https://developer-cn.pico-interactive.com/en/document/unreal/identity-and-friend/)         | Identity & Friend service enables you to access the information of user accounts, get the friend list of the current logged-in users, and more. It also enables your app's users to enjoy a social experience, such as sending friend requests. For more information, refer to Unreal documentation: [Identity Interface](https://docs.unrealengine.com/4.27/en-US/ProgrammingAndScripting/Online/IdentityInterface/) and [Friends Interface](https://docs.unrealengine.com/4.27/en-US/ProgrammingAndScripting/Online/FriendsInterface/).     |
| [RTC](https://developer-cn.pico-interactive.com/en/document/unreal/rtc/)                 |The RTC module in the SDK Platform Service has a centralized communication structure instead of an end-to-end one. Therefore, once the user enters the room and enables voice chat, the microphone continuously captures audio data and uploads it to the RTC server. The RTC server then transmits the audio data to each client in the room, and the client that receives the audio data will broadcast it.      |
| [Room & Matchmaking](https://developer-cn.pico-interactive.com/en/document/unreal/matchmaking/)            |          Room & Matchmaking module offers developers with abilities such as player-to-player networking, matchmaking, room management, and in-room messaging.     |
|  [Social Interaction](https://developer-cn.pico-interactive.com/en/document/unreal/interaction/)                               | Social interaction enables users to enjoy your app with their friends and share their experiences on social platforms.  |
| [Leaderboards](https://developer-cn.pico-interactive.com/en/document/unreal/leaderboard/)                                          | This capability allows for the multi-dimensional display of user game rankings, fostering entertaining competition between players to enhance player engagement. This document introduces the scenarios, features, and concepts related to the leaderboard service.       |
| [Achievement](https://developer-cn.pico-interactive.com/en/document/unreal/achievements/) | Helps to build a positive feedback mechanism in the game, allowing players to obtain achievements within the game, thus enhancing the fun and player engagement. |
|  [Challenge](https://developer-cn.pico-interactive.com/en/document/unreal/challenges/)                             | Challenges belong to leaderboards. You MUST create challenges for leaderboards and manage challenge-related data such as challenge entries.    |
|  [IAP](https://developer-cn.pico-interactive.com/en/document/unreal/in-app-purchase/)     | IAP module integrates various payment systems such as Credit or Debit Card, Paypal, and Alipay, thereby providing a one-stop and flexible payment solution.|
| [DLC](https://developer-cn.pico-interactive.com/en/document/unreal/downloadable-content/)      | DLC provides a more flexible and lighter way to update apps. Once you need to update the content of a published app, you don't need to upload a new version; simply associate new resources (such as levels and cosmetics) as DLCs with the add-ons on the PICO Developer Platform. This way, users can purchase, download and experience the latest game resources without having to upgrade or reinstall your app; DLC also provides you with a broader revenue stream.              |
|  [Subscription](https://developer-cn.pico-interactive.com/document/unreal/subscription/)          | Subscriptions provide a recurring payment model that allows users to purchase the premium content in your app. PICO provides auto-renewable subscriptions. After integrating the Subscriptions service into your app, the order fulfillment and deduction processes are automatically done by the PICO system. |
| [Exercise data authorization](https://developer-cn.pico-interactive.com/en/document/unreal/exercise-data-authorization/)        | Sport service provides multiple APIs to access the user's exercise data from the built-in PICO app — PICO Fitness.         |
|  [Cloud storage](https://developer-cn.pico-interactive.com/en/document/unreal/cloud-storage/)   | Cloud storage is used to back up users' app data, such as identities, custom settings, preference settings, and game progress, on specific devices. If users use new devices, reset their devices, or reinstall apps, the backup data can then be restored to new devices.    |