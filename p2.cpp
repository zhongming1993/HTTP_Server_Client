//
//  p2.cpp
//  test
//
//  Created by MingZhong on 10/24/16.
//  Copyright © 2016 MingZhong. All rights reserved.
//
// you can use includes, for example:
// #include <algorithm>
#include <vector>
#include <string>
using namespace std;
// define an element which contain the name and depth of a directory
struct element{
    string dir;
    int depth;
    element(string my_dir, int my_depth):dir(my_dir), depth(my_depth) {}
};

// parse the string, return the depth of the parsed_result; return -1 if there is no more element in the string
int parse_string(string &S, int& index, string &parsed_result)
{
    int start_index = index;
    string result;
    int depth = 0;
    int size = S.size();
    if (start_index < size)
    {
        while (start_index < size && S[start_index] != '\n')
        {
            if (S[start_index] == '\t')
                depth++;
            else
                parsed_result += S[start_index];
            start_index++;
        }
        index = start_index + 1;
        return depth;
    }
    else
        return -1;
}

// check if a string is a valid name for an image file
bool is_image(string &file_name)
{
    string image_ext1 = ".jpeg";
    string image_ext2 = ".png";
    string image_ext3 = ".gif";
    size_t found1 = file_name.find(image_ext1);
    size_t found2 = file_name.find(image_ext2);
    size_t found3 = file_name.find(image_ext3);
    if ((found1 != string::npos && found1 == (file_name.size() - image_ext1.size())) || (found2 != string::npos && found2 == (file_name.size() - image_ext2.size())) || (found3 != string::npos && found3 == (file_name.size() - image_ext3.size())))
        return true;
    else
        return false;
}

int solution(string &S) {
    if (S == "")
        return 0;
    // record the largest solution of the problem
    int max_length = 0;
    int index = 0;
    string parsed_result;
    // store the upstream directories and their depth
    vector<element> my_vec;
    string revised_string = S + '\n';
    int depth = parse_string(revised_string, index, parsed_result);
    while (depth >= 0)
    {
        if (my_vec.empty())
        {
            if (parsed_result.find('.') == string::npos)
            {
                element current_element(parsed_result, depth);
                my_vec.push_back(current_element);
            }
        }
        else
        {
            element last_element = my_vec.back();
            while (last_element.depth >= depth)
            {
                my_vec.pop_back();
                if (my_vec.empty())
                    break;
                else
                    last_element = my_vec.back();
            }
            if (is_image(parsed_result))
            {
                int current_length = 0;
                for (int i = 0; i < my_vec.size(); i++)
                    current_length += (my_vec[i].dir.size() + 1);
                max_length = max(max_length, current_length);
            }
            else if (parsed_result.find('.') == string::npos)
            {
                element current_element(parsed_result, depth);
                my_vec.push_back(current_element);
            }
        }
        parsed_result = "";
        depth = parse_string(revised_string, index, parsed_result);
    }
    return max_length;
}

/*
#include <iostream>
#include <string>
#include <vector>

using namespace std;

struct element{
    string dir;
    int depth;
    element(string my_dir, int my_depth):dir(my_dir), depth(my_depth){}
};

// parse the input string from start_index, return the depth of the element, if there is no element in the string, return -1;
// index initially hold the start_index of the string, and finally updated with the next avaible position
// parsed_result starts with "" and end with the parsed element
int parse_string(string& input, int& index, string &parsed_result)
{
    // parse the string until we find next "\n"
    int start_index = index;
    string result;
    int depth = 0;
    if (start_index < input.size())
    {
        while (start_index < input.size() && input[start_index] != '\n')
        {
            if (input[start_index] == '\t')
                depth++;
            else parsed_result += input[start_index];
            start_index++;
        }
        index = start_index+1;
        return depth;
    }
    else return -1; // at the end of input
}

bool is_image(string& filename)
{
    string image = ".img";
    int found = filename.find(image);
    if (found != string::npos && found == (filename.size() - image.size()))
        return true;
    else return false;
}

int lengthLongestPath(string input) {
    if (input == "")
        return 0;
    int max_length = 0;
    int index = 0;
    string parsed_result;
    vector<element> my_vec; // stores the directory and it's depth
    string revised_string = input + '\n';
    int depth = parse_string(revised_string, index, parsed_result);
    while (depth >= 0)
    {
        if (my_vec.empty())
        {
            if (parsed_result.find('.') == string::npos)
            {
                element current_element(parsed_result, depth);
                my_vec.push_back(current_element);
            }
        }
        else // vector has something
        {
            element last_element = my_vec.back();
            while (last_element.depth >= depth)
            {
                my_vec.pop_back();
                if (my_vec.empty())
                    break;
                else
                    last_element = my_vec.back();
            }
            // so far, all of the remaining elements in the stack are upstream dirs.
            if (is_image(parsed_result))
            {
                // calculate the path stored in the stack, record the length and update max_length.
                int current_length = 0;
                for (int i = 0; i < my_vec.size(); i++)
                    current_length += (my_vec[i].dir.size() + 1);
                // should contain '\' or not at the end
                max_length = max(max_length, current_length);
            }
            else if (parsed_result.find('.') == string::npos)
            {
                element current_element(parsed_result, depth);
                my_vec.push_back(current_element);
            }
        }
        parsed_result = "";
        depth = parse_string(revised_string, index, parsed_result);
    }
    return max_length;
}

*/
int main()
{
    string input = "dir\n\tsubdir1\n\t\timg.imrg\n\t\tsubdir2\n\t\t\timg.imge\n\t\t\timgqqqqqqqqqqqqqqqqqqqqqqqq.imge\n\tsubdir3333333333333\n\t\timg.imag\n\tsubdir33333333333333\n\t\timg.ext";
    cout << solution(input) << endl;
}

