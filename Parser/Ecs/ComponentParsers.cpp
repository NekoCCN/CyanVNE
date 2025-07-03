//
// Created by Ext Culith on 2025/7/3.
//

#include "ComponentParsers.h"
#include "Runtime/Components/Components.h"
#include "Runtime/ICommand/ICommand.h"

namespace cyanvne::parser::ecs
{
   std::unique_ptr<IComponentEmplacer> TransformComponentParser::parse(const YAML::Node& node, const NodeParserRegistry&) const {
        cyanvne::ecs::TransformComponent component;
        if (node["position"])
        {
            auto vec = util::getSequenceAsVectorElseThrow<float>(node, "position", "TransformComponent");
            if (vec.size() != 2)
                throw exception::parserexception::ParserException("Parse Error", "position must be a sequence of 2 numbers.");
            component.position = { vec[0], vec[1] };
        }
        if (node["scale"])
        {
            auto vec = util::getSequenceAsVectorElseThrow<float>(node, "scale", "TransformComponent");
            if (vec.size() != 2)
                throw exception::parserexception::ParserException("Parse Error", "scale must be a sequence of 2 numbers.");
            component.scale = { vec[0], vec[1] };
        }
        return std::make_unique<ComponentEmplacer<cyanvne::ecs::TransformComponent>>(component);
    }

    std::unique_ptr<IComponentEmplacer> LayoutComponentParser::parse(const YAML::Node& node, const NodeParserRegistry&) const
    {
        cyanvne::ecs::LayoutComponent component;
        if (node["area_ratio"])
        {
            auto vec = util::getSequenceAsVectorElseThrow<float>(node, "area_ratio", "LayoutComponent");
            if (vec.size() != 4)
                throw exception::parserexception::ParserException("Parse Error", "area_ratio must be a sequence of 4 numbers.");
            component.area_ratio = { vec[0], vec[1], vec[2], vec[3] };
        }
        if (node["anchor"])
        {
            auto vec = util::getSequenceAsVectorElseThrow<float>(node, "anchor", "LayoutComponent");
            if (vec.size() != 2) throw exception::parserexception::ParserException("Parse Error", "anchor must be a sequence of 2 numbers.");
            component.anchor = { vec[0], vec[1] };
        }
        return std::make_unique<ComponentEmplacer<cyanvne::ecs::LayoutComponent>>(component);
    }

    std::unique_ptr<IComponentEmplacer> SpriteComponentParser::parse(const YAML::Node& node, const NodeParserRegistry&) const
    {
        cyanvne::ecs::SpriteComponent component;
        component.resource_key = util::getScalarNodeElseThrow<std::string>(node, "resource_key", "SpriteComponent");
        if (node["source_rect"])
        {
            auto rect_vec = util::getSequenceAsVectorElseThrow<float>(node, "source_rect", "SpriteComponent");
            if (rect_vec.size() != 4)
                throw exception::parserexception::ParserException("Parse Error", "source_rect must be a sequence of 4 numbers.");
            component.source_rect = { rect_vec[0], rect_vec[1], rect_vec[2], rect_vec[3] };
        }
        return std::make_unique<ComponentEmplacer<cyanvne::ecs::SpriteComponent>>(component);
    }

    std::unique_ptr<IComponentEmplacer> SpriteAnimationComponentParser::parse(const YAML::Node& node, const NodeParserRegistry&) const
    {
        cyanvne::ecs::SpriteAnimationComponent component;
        component.frame_duration = util::getScalarNodeElseThrow<float>(node, "frame_duration", "SpriteAnimation");
        component.loop = util::getScalarNodeElseThrow<bool>(node, "loop", "SpriteAnimation");

        auto frames_node = util::getSequenceAsNodeVectorElseThrow(node, "frames", "SpriteAnimation");
        for(const auto& frame_node : frames_node)
        {
            auto frame_vec = frame_node.as<std::vector<float>>();
            if (frame_vec.size() != 4)
                throw exception::parserexception::ParserException("Parse Error", "animation frame must be a sequence of 4 numbers.");
            component.frames.push_back({ frame_vec[0], frame_vec[1], frame_vec[2], frame_vec[3] });
        }
        return std::make_unique<ComponentEmplacer<cyanvne::ecs::SpriteAnimationComponent>>(component);
    }

    static std::vector<std::unique_ptr<cyanvne::ecs::commands::ICommand>> parse_command_list(const YAML::Node& node, const NodeParserRegistry& main_registry)
    {
        std::vector<std::unique_ptr<cyanvne::ecs::commands::ICommand>> commands;
        if (!node.IsSequence())
        {
            throw exception::parserexception::ParserException("Command Parse Error", "Expected a sequence of commands.");
        }
        for (const auto& cmd_node : node)
        {
            auto parsed_data = main_registry.parseNodeFromTypeField(cmd_node);

            if (const auto* cmd = parsed_data->getAs<std::unique_ptr<cyanvne::ecs::commands::ICommand>>())
            {
                commands.push_back((*cmd)->clone());
            }
        }
        return commands;
    }

    std::unique_ptr<IComponentEmplacer> ClickableComponentParser::parse(const YAML::Node& node, const NodeParserRegistry& main_registry) const
    {
        cyanvne::ecs::ClickableComponent component;
        if (node["on_left_click"])
        {
            auto cmd_list = parse_command_list(node["on_right_click"], main_registry);

            for (const auto& cmd : cmd_list)
            {
                if (!cmd)
                {
                    throw exception::parserexception::ParserException("ClickableComponent Parse Error", "Command list contains a null command.");
                }

                component.on_left_click.push_back(cmd->clone());
            }
        }
        if (node["on_right_click"])
        {
            auto cmd_list = parse_command_list(node["on_right_click"], main_registry);

            for (const auto& cmd : cmd_list)
            {
                if (!cmd)
                {
                    throw exception::parserexception::ParserException("ClickableComponent Parse Error", "Command list contains a null command.");
                }

                component.on_right_click.push_back(cmd->clone());
            }
        }
        return std::make_unique<ComponentEmplacer<cyanvne::ecs::ClickableComponent>>(std::move(component));
    }

    std::unique_ptr<IComponentEmplacer> KeyFocusComponentParser::parse(const YAML::Node& node, const NodeParserRegistry& main_registry) const
    {
        cyanvne::ecs::KeyFocusComponent component;
        if (!node.IsMap())
        {
            throw exception::parserexception::ParserException("KeyFocus Parse Error", "Expected a map of keycodes to command lists.");
        }
        for (const auto& pair : node)
        {
            SDL_Keycode keycode = SDL_GetKeyFromName(pair.first.as<std::string>().c_str());
            if (keycode == SDLK_UNKNOWN)
            {
                throw exception::parserexception::ParserException("KeyFocus Parse Error", "Invalid key name: " + pair.first.as<std::string>());
            }

            auto cmd_list = parse_command_list(pair.second, main_registry);

            for (const auto& cmd : cmd_list)
            {
                if (!cmd)
                {
                    throw exception::parserexception::ParserException("KeyFocus Parse Error", "Command list contains a null command for key: " + pair.first.as<std::string>());
                }
                component.key_actions[keycode].push_back(cmd->clone());
            }
        }
        return std::make_unique<ComponentEmplacer<cyanvne::ecs::KeyFocusComponent>>(std::move(component));
    }

    std::unique_ptr<IComponentEmplacer> ScrollableComponentParser::parse(const YAML::Node& node, const NodeParserRegistry& main_registry) const
   {
       cyanvne::ecs::ScrollableComponent component;
       if (node["on_scroll_up"])
       {
           auto cmd_list = parse_command_list(node["on_scroll_up"], main_registry);

           for (const auto& cmd : cmd_list)
           {
               if (!cmd)
               {
                   throw exception::parserexception::ParserException("ScrollableComponent Parse Error", "Command list contains a null command.");
               }

               component.on_scroll_up.push_back(cmd->clone());
           }
       }
       if (node["on_scroll_down"])
       {
           auto cmd_list = parse_command_list(node["on_scroll_down"], main_registry);

           for (const auto& cmd : cmd_list)
           {
               if (!cmd)
               {
                   throw exception::parserexception::ParserException("ScrollableComponent Parse Error", "Command list contains a null command.");
               }

               component.on_scroll_down.push_back(cmd->clone());
           }
       }
       return std::make_unique<ComponentEmplacer<cyanvne::ecs::ScrollableComponent>>(std::move(component));
   }
}
