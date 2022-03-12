# BasicFileDropper
Drops a file on the user's computer, allows for program persistence as well

example:

```cpp
int main()
{
    c_dropper payload("https://post.medicalnewstoday.com/wp-content/uploads/sites/3/2020/02/322868_1100-800x825.jpg", "C:\\dog.jpg", true,
        c_dropper::persistence_t(true, "dog.jpg","Dog pic", c_dropper::persistence_t::APPDATA));
    payload.drop();

    return 0;
}
```
