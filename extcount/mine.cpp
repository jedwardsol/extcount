#include <filesystem>
namespace fs=std::filesystem;
#include <string>
#include <map>
#include <cstdint>
#include <ranges>
#include <algorithm>

#include "include/print.h"
#include "include/args.h"
#include "include/thrower.h"


struct Stats
{
    int64_t     size{};
    int         count{};

    friend
    auto operator<=>(Stats const &,Stats const &)=default;

    friend
    bool operator==(Stats const &,Stats const &)=default;
};

using Map = std::map<std::string, Stats>;



std::string pretty(int64_t  size)
{
    if(size > 1'000'000'000'000)
    {
        return std::format("{:.2f} Tb", size/1'000'000'000'000.0);
    }
    else if(size > 1'000'000'000)
    {
        return std::format("{:.2f} Gb", size/1'000'000'000.0);
    }
    else if(size > 1'000'000)
    {
        return std::format("{:.2f} Mb", size/1'000'000.0);
    }
    else 
    {
        return std::format("{} bytes", size);
    }
}


void walk(fs::path const &path, Map &extensions)
try
{
    for(auto entry : fs::directory_iterator(path))
    {
        if(entry.is_directory())
        {
            walk(entry.path(),extensions);
        }
        else
        {
            auto extension  = entry.path().extension().string();
            auto size       = entry.file_size();
        
            extensions[extension].count++;
            extensions[extension].size+=size;
        }
    }
}
catch(std::exception const &e)
{
    print("{} : {}\r",path.string(),e.what());
}



void go(fs::path const &path)
{
    Map     extensions;

    walk(path,extensions);

    print("\n");

    const auto deConstKey = [](auto kv) 
    {
        auto &[key,value] = kv;
        return std::make_pair(key,value);
    };

    auto sortable = std::ranges::to<std::vector>(extensions | std::views::transform(deConstKey));

    std::ranges::sort(sortable,std::greater<>{},   & decltype(sortable)::value_type::second);

    for(auto const &[extension, stats] : sortable)
    {
        std::print("{:<40} : {:>20} in {} files\n",extension,pretty(stats.size),stats.count);
    }
}


int main(int argc, char **argv)
try
{
    Args    args{argc,argv};

    if(args.empty())
    {
        go(fs::current_path());
    }
    else
    {
        go(args[0]);
    }

}
catch(std::exception const &e)
{
    print("{}",e.what());
}


